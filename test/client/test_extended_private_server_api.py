import json

import numpy as np
import pytest


def test_plus_commands_are_separate_from_open_source_api():
    from unrealcv.api import UnrealCv_API
    from unrealcv.plus_api import UnrealCvPlusAPI

    assert not hasattr(UnrealCv_API, 'mount_pak')
    assert not hasattr(UnrealCv_API, 'get_scene_occupancy')
    assert hasattr(UnrealCvPlusAPI, 'mount_pak')
    assert hasattr(UnrealCvPlusAPI, 'get_scene_occupancy')


def test_get_scene_occupancy_decodes_bool_npy(make_npy_bytes, dummy_client_factory, plus_api_factory):
    payload = make_npy_bytes(np.zeros((2, 3, 4), dtype=bool))
    client = dummy_client_factory([payload])
    api = plus_api_factory(client)

    result = api.get_scene_occupancy()

    assert result.shape == (2, 3, 4)
    assert result.dtype == np.dtype(bool)
    assert client.calls[-1] == ('vget /scene/occupancy lingo_vis bounds', (120,))


def test_scene_occupancy_transform_command(plus_api_factory):
    api = plus_api_factory()
    command = api._build_scene_occupancy_command(
        'vget /scene/occupancy', 'lingo_train', 'mesh', (1, 2, 3), 45, True
    )
    assert command == 'vget /scene/occupancy lingo_train mesh 1.0 2.0 3.0 45.0 1'


def test_scene_occupancy_spec_decodes_json(dummy_client_factory, plus_api_factory):
    client = dummy_client_factory([json.dumps({'profile': 'lingo_vis', 'method': 'mesh'})])
    api = plus_api_factory(client)
    assert api.get_scene_occupancy_spec(method='mesh')['method'] == 'mesh'
    assert client.calls[-1] == ('vget /scene/occupancy/spec lingo_vis mesh', ())


@pytest.mark.parametrize(
    'method_name,modality',
    [
        ('capture_panoramic_normal', 'normal'),
        ('capture_panoramic_mask', 'mask'),
        ('capture_panoramic_depth', 'depth'),
    ],
)
def test_panoramic_modality_commands(plus_api_factory, method_name, modality):
    api = plus_api_factory()
    command = getattr(api, method_name)(0, 'out.png', 2048, 1024, return_cmd=True)
    assert command == f'vget /camera/0/panoramic/{modality} out.png 2048 1024'
