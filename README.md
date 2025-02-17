# EldenRingBossState

这是一个基于modengine2的艾尔登法环mod，用于读取锁定的敌人信息，并显示在ImGui窗口上，灵感和实现方法启发于Hexinton团队制作，SilverCelty汉化的法环ct表，在此感谢！

如果你想要添加敌人的更多信息，可以在ct表上查询相应的地址和偏移量，LastLockOnTarget的值即是g_raxValue的值，具体实现参考Run的内容。

## 使用

下载最新的Release，解压并将解压出来的所有文件及文件夹放到法环根目录，修改config_eldenring.toml，具体修改方式如下

将第14行（可能有变动）的external_dlls添加一项`"BossState\\BossState.dll"`。

例如，如果原来第14行为`external_dlls = ["A.dll", "B\\B.dll", ...]`

则修改后为：`external_dlls = ["A.dll", "B\\B.dll", ..., "BossState\\BossState.dll"]`

如果原来为`external_dlls = []`

则修改后为：`external_dlls = ["BossState\\BossState.dll"]`

随后启动`launchmod_eldenring.bat`，如果看到出现了一个新的窗口，则代表成功。

## 编译

运行BuildWindows.bat编译项目，具体运行参考Release的文件结构，需要使用modengine2将dll注入到游戏中（如果你会其它方法也可以，但是我认为这是最简单有效且广泛适用的）

## 使用的第三方库

ImGui, GLEW, GLFW, MinHook

关于[第三方库开源协议许可](./TrdLicense/)详见链接内目录，位于TrdLicense文件夹内