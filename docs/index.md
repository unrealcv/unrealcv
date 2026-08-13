# UnrealCV

[![Join the chat at https://gitter.im/unrealcv/unrealcv](https://badges.gitter.im/unrealcv/unrealcv.svg)](https://gitter.im/unrealcv/unrealcv?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Docs Status](https://readthedocs.org/projects/unrealcv/badge/?version=latest
)](http://docs.unrealcv.org)

<!-- [![Build Status](https://travis-ci.org/unrealcv/unrealcv.svg?branch=master)](https://travis-ci.org/unrealcv/unrealcv) -->

UnrealCV is a project to help computer vision researchers build virtual worlds using Unreal Engine (UE). It extends UE with a plugin by providing:

1. A set of UnrealCV commands to interact with the virtual world.
2. Communication between UE and an external program, such as PyTorch or TensorFlow.

UnrealCV can be used in two ways. The first is using a compiled game binary with UnrealCV embedded. The second is installing the UnrealCV plugin in an Unreal Engine project and using the editor to build a new virtual world.


Please read [Tutorial: Getting Started](http://unrealcv.github.io/tutorial/getting_started.html) to learn using UnrealCV.

## UnrealCV Dev For [UnrealZoo](https://github.com/UnrealZoo)

{doc}`UnrealCV Dev For <unrealcv_plus/index>` [UnrealZoo](https://github.com/UnrealZoo)
contains features in active development and testing. They are currently provided
in supported [UnrealZoo](https://github.com/UnrealZoo) environments before general-purpose
parts are promoted into the open-source plugin. Runtime MCP clients, examples,
and an agent skill are published at
[unrealcv-runtime-mcp](https://github.com/lizi-Margin/unrealcv-runtime-mcp);
the Unreal Engine C++ Runtime MCP server is not currently open source.

<center>
<img src="http://unrealcv.github.io/images/homepage_teaser.png" alt="annotation"/>
Images generated from the technical demo <a href="http://docs.unrealcv.org/en/master/reference/model_zoo.html#realisticrendering">RealisticRendering</a><br>
</center>

## Citation

If you found this project useful, please consider citing our paper

```bibtex
@article{qiu2017unrealcv,
  Author = {Weichao Qiu, Fangwei Zhong, Yi Zhang, Siyuan Qiao,Zihao Xiao, Tae Soo Kim, Yizhou Wang, Alan Yuille},
  Journal = {ACM Multimedia Open Source Software Competition},
  Title = {UnrealCV: Virtual Worlds for Computer Vision},
  Year = {2017}
}
```

## Contact
If you have any suggestion or interested in using UnrealCV, please [contact us](http://unrealcv.github.io/contact.html).
