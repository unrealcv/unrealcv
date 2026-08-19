"""Render a multi-view image from occupancy NPY exported by the region API."""

from __future__ import annotations

import argparse

import matplotlib.pyplot as plt
import numpy as np


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("occupancy")
    parser.add_argument("output")
    parser.add_argument("--min-m", nargs=3, type=float, required=True)
    parser.add_argument("--voxel-size-m", type=float, required=True)
    parser.add_argument("--max-points", type=int, default=250_000)
    args = parser.parse_args()

    grid = np.asarray(np.load(args.occupancy, allow_pickle=False), dtype=bool)
    if grid.ndim != 3:
        raise ValueError(f"expected a 3-D occupancy array, got shape {grid.shape}")
    points = np.argwhere(grid)
    if len(points) > args.max_points:
        points = points[::int(np.ceil(len(points) / args.max_points))]
    xyz = np.asarray(args.min_m) + (points + 0.5) * args.voxel_size_m
    height = xyz[:, 1]

    fig = plt.figure(figsize=(16, 12), constrained_layout=True)
    for index, (elev, azim) in enumerate(((25, -60), (25, 35), (65, -35), (10, 140)), 1):
        axis = fig.add_subplot(2, 2, index, projection="3d")
        axis.scatter(xyz[:, 0], xyz[:, 2], xyz[:, 1], c=height,
                     cmap="turbo", s=1, linewidths=0, alpha=0.75)
        axis.set(xlabel="X (m)", ylabel="Z (m)", zlabel="Y up (m)")
        axis.view_init(elev=elev, azim=azim)
        axis.set_title(f"Occupancy region view {index}")
    fig.suptitle(f"UnrealCV Dev for UnrealZoo occupancy region - {grid.shape} voxels")
    fig.savefig(args.output, dpi=180)
    plt.close(fig)


if __name__ == "__main__":
    main()
