"""Custom exception hierarchy for the unrealcv Python client.

All exceptions inherit from :class:`UnrealCVError` so callers can catch
the entire family with a single ``except UnrealCVError`` clause.
"""


class UnrealCVError(Exception):
    """Base exception for all unrealcv client errors."""


class ConnectionError(UnrealCVError):
    """Raised when a connection to the UnrealCV server cannot be established or is lost."""


class DisconnectedError(ConnectionError):
    """Raised when an operation is attempted on a disconnected client."""


class MessageError(UnrealCVError):
    """Raised when a received message is malformed or cannot be parsed."""


class RequestError(UnrealCVError):
    """Raised when sending a request to the server fails."""


class TimeoutError(UnrealCVError):
    """Raised when a server response is not received within the timeout window."""
