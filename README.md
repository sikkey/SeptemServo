# SeptemServo
plugin version of SeptemServoPro , Alpha

# Usage

##Usage (English):

1. (Optional)Create a new UE4 project
2. open powershell in windows or sh in linux
3. cd the directory of the project
4. mkdir ./Plugins/
5. cd ./Plugins/
6. git submodule add https://github.com/sikkey/SeptemServo.git SeptemServo
	(Makesure the directory name is SeptemServo)
7. generate vs project files
8. build vs project
9. reopen ue4 project & rebuild in engine

###Update Plugin Code

1. git pull --recurse-submodules
2. git submodule update --remote --recursive

###Resolve submodule dirty

1. delete plugin directory ./SeptemServo/
	linux: rm -rf ./SeptemServo/
2. git submodule init
3. git submodule update
4. generate vs project files, rebuild in vs, then rebuild in ue4


##使用说明 (中文):

1. (可选)创建或选择一个UE4工程
2. win10打开powershell，或者在linux下使用命令行
3. cd进入项目目录
4. (可选)创建Plugins目录	mkdir ./Plugins/
5. 进入Plugins目录	cd ./Plugins/
6. 运行命令行，添加git子模组(确保子模组文件夹名为SeptemServo)
	git submodule add https://github.com/sikkey/SeptemServo.git SeptemServo
7. 右键.uproject文件，重新生成VS工程文件
8. 重新编译VS工程
9. 重新打开UE4工程，并重新在引擎编辑器中测试或重编译项目

###更新插件源码

1. git pull --recurse-submodules
2. git submodule update --remote --recursive

###解决子模块脏数据

1. 删除插件SeptemServo文件夹
	linux: rm -rf ./SeptemServo/
打开powershell或shell
2. git submodule init
3. git submodule update
4. 重新生成、编译VS工程。重新编译ue4项目

###删除模块
```
git submodule deinit SeptemServo
```
###删除.gitmodules中记录的模块信息（--cached选项清除.git/modules中的缓存）
```
git rm --cached SeptemServo
```
