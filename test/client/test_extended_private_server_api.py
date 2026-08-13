import json

import numpy as np
import pytest

def test_get_scene_occupancy_decodes_bool_npy(make_npy_bytes, dummy_client_factory, api_factory):
    payload = make_npy_bytes(np.zeros((2, 3, 4), dtype=bool))
    client = dummy_client_factory([payload])
    api = api_factory(client)

    result = api.get_scene_occupancy()

    assert result.shape == (2, 3, 4)
    assert result.dtype == np.dtype(bool)
    assert client.calls[-1] == ('vget /scene/occupancy lingo_vis bounds', (120,))


def test_scene_occupancy_transform_command(api_factory):
    api = api_factory()
    command = api._build_scene_occupancy_command(
        'vget /scene/occupancy', 'lingo_train', 'mesh', (1, 2, 3), 45, True
    )
    assert command == 'vget /scene/occupancy lingo_train mesh 1.0 2.0 3.0 45.0 1'


def test_scene_occupancy_spec_decodes_json(dummy_client_factory, api_factory):
    client = dummy_client_factory([json.dumps({'profile': 'lingo_vis', 'method': 'mesh'})])
    api = api_factory(client)
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
def test_panoramic_modality_commands(api_factory, method_name, modality):
    api = api_factory()
    command = getattr(api, method_name)(0, 'out.png', 2048, 1024, return_cmd=True)
    assert command == f'vget /camera/0/panoramic/{modality} out.png 2048 1024'
