"""Live smoke test for the UnrealCV Cine camera API."""

import argparse
import json
from pathlib import Path

from unrealcv.api import UnrealCv_API


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', type=int, default=9000)
    parser.add_argument('--output', type=Path, default=Path('test/cine_camera_live_smoke.json'))
    args = parser.parse_args()
    api = UnrealCv_API(args.port, args.host, (640, 480))
    rows = []
    calls = [
        ('get_cine_camera', (0,)),
        ('get_cine_camera_enabled', (0,)),
        ('get_cine_intrinsics', (0,)),
        ('set_cine_camera_enabled', (0, True)),
        ('set_cine_filmback', (0, 36, 24, 0, 0)),
        ('set_cine_lens', (0, 50, 2.8)),
        ('set_cine_lens_settings', (0, 20, 200, 1.4, 16, 10, 1, 6)),
        ('set_cine_focus', (0, 100)),
        ('set_cine_focus_mode', (0, 'manual', False, 2, 0)),
        ('set_cine_crop', (0, 1.777, 0, False, False)),
        ('set_cine_near_clip', (0, False, 10)),
        ('set_cine_exposure', (0, 100, 120, False)),
    ]
    for name, call_args in calls:
        row = {'function': name, 'command': getattr(api, name)(*call_args, return_cmd=True)}
        try:
            result = getattr(api, name)(*call_args)
            row.update(status='PASS', result=repr(result)[:500])
        except Exception as exc:  # pragma: no cover - live server diagnostics
            row.update(status='FAIL', error=f'{type(exc).__name__}: {exc}')
        rows.append(row)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(rows, indent=2), encoding='utf-8')
    print(json.dumps({
        'functions': len(rows),
        'pass': sum(row['status'] == 'PASS' for row in rows),
        'fail': sum(row['status'] == 'FAIL' for row in rows),
        'output': str(args.output),
    }, indent=2))


if __name__ == '__main__':
    main()
