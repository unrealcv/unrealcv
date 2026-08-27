import json


def test_cine_getters_decode_and_build_commands(dummy_client_factory, api_factory):
    client = dummy_client_factory([
        json.dumps({"enabled": True}),
        "1",
        json.dumps({"width": 640, "height": 480}),
    ])
    api = api_factory(client)

    assert api.get_cine_camera(0) == {"enabled": True}
    assert api.get_cine_camera_enabled(0) is True
    assert api.get_cine_intrinsics(0) == {"width": 640, "height": 480}
    assert client.calls == [
        ('vget /camera/0/cine', ()),
        ('vget /camera/0/cine/enabled', ()),
        ('vget /camera/0/cine/intrinsics', ()),
    ]


def test_cine_setters_use_handler_argument_order(api_factory):
    api = api_factory()
    cases = [
        ('set_cine_camera_enabled', (0, True), 'vset /camera/0/cine/enabled 1'),
        ('set_cine_filmback', (0, 36, 24, 1, 2),
         'vset /camera/0/cine/filmback 36 24 1 2'),
        ('set_cine_lens', (0, 50, 2.8), 'vset /camera/0/cine/lens 50 2.8'),
        ('set_cine_lens_settings', (0, 20, 200, 1.4, 16, 10, 1, 6),
         'vset /camera/0/cine/lens_settings 20 200 1.4 16 10 1 6'),
        ('set_cine_focus', (0, 100), 'vset /camera/0/cine/focus 100'),
        ('set_cine_focus_mode', (0, 'manual', True, 2, 0.5),
         'vset /camera/0/cine/focus_mode manual 1 2 0.5'),
        ('set_cine_tracking_focus', (0, 'Target', 1, 2, 3),
         'vset /camera/0/cine/focus_tracking Target 1 2 3'),
        ('set_cine_crop', (0, 1.777, 0.1, True, False),
         'vset /camera/0/cine/crop 1.777 0.1 1 0'),
        ('set_cine_near_clip', (0, True, 5),
         'vset /camera/0/cine/near_clip 1 5'),
        ('set_cine_exposure', (0, 100, 120, False),
         'vset /camera/0/cine/exposure 100 120 0'),
    ]
    for method_name, args, expected in cases:
        assert getattr(api, method_name)(*args, return_cmd=True) == expected
