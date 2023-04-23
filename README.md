# CADSimplifier

#### 介绍
一个FreeCAD里的workbench,可以实现自动寻找并删除模型的圆角

#### 软件架构
软件架构说明

#### 安装教程
1.fork或者git下载本项目源文件。
2.将下载的整个CADSimplifier文件夹拷贝添加到yourFreeCAD/Mod文件夹下，
结果为yourFreeCAD/Mod/CADSimplifier
如：E:\source\freecad\FreeCAD-master\src\Mod路径下添加拷贝的文件夹。
3.在yourFreeCAD/Mod文件夹下的同级目录的CMakeLists.txt文件里添加一句
   add_subdirectory(CADSimplifier)，接着用cmake重新生成project即可
#### 使用说明
1 启动FreeCAD
2 打开一个新模型文件
3 切换到CADSimplifier工具台
4 CADSimplifier工具台提供了一个工具栏和菜单。点击工具栏里的删除圆角图标进行测试

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
