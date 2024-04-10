# librime-cloud: RIME 云输入插件

## 获取
  免责声明：下述二进制文件是使用 GitHub CI，引用公开获取的源代码和二进制程序自动生成的，仅供参考。
  请了解运行未知来源二进制文件的风险。使用下述文件造成的后果请您自行承担，制作者不承担任何责任。

  [GitHub Release](https://github.com/hchunhui/librime-cloud/releases)

  您也可以从 https://github.com/hchunhui/librime-cloud 获取编译脚本，从源码编译。

## 安装
  1. 编译或获取压缩包
  2. 解压
     - Windows 平台（小狼毫 >= 0.14.0）
       - 将 `out-mingw` 下所有文件复制到小狼毫的程序文件夹下
       - 将 `scripts` 下所有文件复制到小狼毫的用户目录下
     - Linux 平台（librime 需编译 lua 支持）
       - 将 `out-linux` 下所有文件复制到 `/usr/local/lib/lua/$LUAV` 下
       - 将 `scripts` 下所有文件复制到用户目录下
     - macOS 平台（小企鹅）
       - 将 `out-macos` 下所有文件复制到 `/usr/local/lib/lua/$LUAV` 下
       - 将 `scripts` 下所有文件复制到 `~/.local/share/fcitx5/rime` 下
  3. 配置：见 `scripts/rime.lua` 中的注释

## 使用
  默认情况下，在输入状态下按 `Control+t` 触发一次云输入，云候选前五项自动加到候选菜单最前方。
