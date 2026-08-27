import re
import warnings

__all__ = ['ApiVersionManager']


class ApiVersionManager:
    """Track server version and command availability for an UnrealCV client."""

    COMMANDS_QUERY = 'vget /unrealcv/commands'
    VERSION_QUERY = 'vget /unrealcv/version'
    COMMAND_CAPABILITY_MIN_VERSION = (1, 1, 0)
    UNREALCV_PLUS_MIN_VERSION = (2, 0, 0)

    def __init__(self, request):
        self._request = request
        self._server_version = None
        self._server_version_checked = False
        self._supported_command_templates = None
        self._command_capabilities_checked = False
        self._capability_warning_emitted = False
        self._unrealcv_plus_warning_emitted = False

    @staticmethod
    def normalize_command(command):
        if isinstance(command, bytes):
            command = command.decode('utf-8', errors='replace')
        return ' '.join(str(command).split())

    @classmethod
    def matches_command_template(cls, command, template):
        command = cls.normalize_command(command)
        template = cls.normalize_command(template)
        if not command or not template:
            return False

        pattern_parts = []
        position = 0
        for placeholder in re.finditer(r'\[[^\[\]]+\]', template):
            literal = template[position : placeholder.start()]
            pattern_parts.append(re.escape(literal).replace(r'\ ', r'\s+'))
            pattern_parts.append(r'\S*')
            position = placeholder.end()
        literal = template[position:]
        pattern_parts.append(re.escape(literal).replace(r'\ ', r'\s+'))
        return re.fullmatch(''.join(pattern_parts), command) is not None

    @staticmethod
    def parse_version_tuple(version):
        if isinstance(version, bytes):
            version = version.decode('utf-8', errors='replace')
        if not isinstance(version, str):
            return None
        parts = re.findall(r'\d+', version)
        if not parts:
            return None
        version_parts = [int(part) for part in parts[:3]]
        version_parts.extend([0] * (3 - len(version_parts)))
        return tuple(version_parts)

    def load(self, timeout=5):
        """Load version and command metadata once for the connected server."""
        self.load_command_capabilities(timeout=timeout)
        if self.has_command_capabilities():
            self.get_server_version(timeout=timeout)
        else:
            self._server_version_checked = True

    def get_server_version(self, timeout=5):
        if not self._server_version_checked:
            self._server_version_checked = True
            try:
                response = self._request(self.VERSION_QUERY, timeout=timeout)
            except (ConnectionError, TimeoutError):
                response = None
            if isinstance(response, bytes):
                response = response.decode('utf-8', errors='replace')
            if isinstance(response, str) and not response.lstrip().lower().startswith('error'):
                self._server_version = response.strip()
        return self._server_version

    def get_server_version_tuple(self, timeout=5):
        return self.parse_version_tuple(self.get_server_version(timeout=timeout))

    def get_supported_command_templates(self):
        return self._supported_command_templates

    def command_capabilities_checked(self):
        return self._command_capabilities_checked

    def is_loaded(self):
        return self._server_version_checked and self._command_capabilities_checked

    def has_command_capabilities(self):
        return self._supported_command_templates is not None

    def load_command_capabilities(self, timeout=5):
        self._command_capabilities_checked = True
        try:
            response = self._request(self.COMMANDS_QUERY, timeout=timeout)
        except (ConnectionError, TimeoutError):
            self._warn_command_capabilities_unavailable()
            return

        if isinstance(response, bytes):
            response = response.decode('utf-8', errors='replace')
        if not isinstance(response, str) or response.lstrip().lower().startswith('error'):
            self._warn_command_capabilities_unavailable()
            return

        templates = tuple(
            self.normalize_command(line)
            for line in response.splitlines()
            if self.normalize_command(line)
        )
        if not templates or self.COMMANDS_QUERY not in templates:
            self._warn_command_capabilities_unavailable()
            return

        self._supported_command_templates = templates

    def supports_command(self, command):
        """Return True/False when known, or None when the server cannot report commands."""
        if self._supported_command_templates is None:
            return None
        return any(
            self.matches_command_template(command, template)
            for template in self._supported_command_templates
        )

    def supports_shared_command(self, command):
        """Return whether an explicitly selected shared command is supported."""
        return self.supports_command(command) is True

    def is_unrealcv_plus(self):
        version = self.get_server_version_tuple()
        return version is not None and version >= self.UNREALCV_PLUS_MIN_VERSION

    def warn_if_command_maybe_unsupported(self, command, stacklevel=3):
        if self.supports_command(command) is not False:
            return
        normalized = self.normalize_command(command)
        warnings.warn(
            f"The connected UnrealCV server may not support command '{normalized}'. "
            'The request will still be sent.',
            UserWarning,
            stacklevel=stacklevel,
        )

    def warn_unrealcv_plus_if_unsupported(self, stacklevel=2):
        if self._unrealcv_plus_warning_emitted or self.is_unrealcv_plus():
            return
        warnings.warn(
            'UnrealCV Dev For UnrealZoo APIs require UnrealCV server version >= 2.0.0. '
            f'Current server version from `{self.VERSION_QUERY}` is '
            f'{self.get_server_version()!r}. '
            'Please make sure you are using the latest UnrealCV Dev For UnrealZoo build '
            'to get UnrealCV Dev For UnrealZoo API support.',
            UserWarning,
            stacklevel=stacklevel,
        )
        self._unrealcv_plus_warning_emitted = True

    def _warn_command_capabilities_unavailable(self):
        if self._capability_warning_emitted:
            return
        warnings.warn(
            'The connected UnrealCV server is too old to report its supported commands. '
            'Command capability detection requires UnrealCV server 1.1.0 or newer; '
            'requests will continue without capability checks.',
            UserWarning,
            stacklevel=3,
        )
        self._capability_warning_emitted = True
