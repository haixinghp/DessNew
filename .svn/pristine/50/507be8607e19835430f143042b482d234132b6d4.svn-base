###########环境依赖
MDK525及相关平台依赖package，下载地址：svn://47.104.85.231/edata/文档/平台

###########环境注意事项
请确保在公司的加密环境下下载和上传代码

###########代码编写规范
详见地址：svn://47.104.85.231/edata/部门规范/软件规范/C语言代码编程规范.docx

###########SVN代码提交规范
详见地址：svn://47.104.85.231/edata/部门规范/软件规范/SVN版本提交规范.docx

###########部署步骤
本地部署：
	打开"./horizon/#对应芯片平台#/LOCK/ble_app_lock/mdk/*.uvprojx"工程文件进行编译部署

平台自动化部署：
	打开“192.168.1.198:8080”Jenkins自动化部署平台，登录后选择项目进行平台自动构建部署

###########目录结构描述
├── readme.md			// 帮助文档
├── main.c			// main
├── chip                         		// 芯片平台目录
│   ├── nRF52832			// nRF52832，厂家nodic，Flash 512KB  RAM 64KB  ，详见文档：svn://47.104.85.231/edata/文档/平台/nrf52832/nRF52832_PS_v1.3[1].pdf
│      └── LOCK
│         └── ble_app_lock
│            └── CHIP
│               └── PlatformFunc.c		// 供main函数调用执行的系统初始化函数及循环执行函数实现源文件
│            └── mdk
│               └── nrf52_lock.uvprojx	// MDK工程文件
│   ├── nRF52833			// nRF52833，厂家nodic，Flash 512KB  RAM 128KB ，详见文档：svn://47.104.85.231/edata/文档/平台/nrf52833/nRF52833_PS_v1.3.pdf
│      └── LOCK
│         └── ble_app_lock
│            └── CHIP
│               └── PlatformFunc.c		// 供main函数调用执行的系统初始化函数及循环执行函数实现源文件
│            └── mdk
│               └── nrf52_lock.uvprojx	// MDK工程文件
│   ├── RTL8762			// RTL8762，厂家realtek，Flash 512KB  RAM 64KB ，详见文档：svn://47.104.85.231/edata/文档/平台/RTL8762C/RTL8762C_Series_Datasheet_0.86(382).pdf
├── 3rdParty			// 第三方库
│   ├── fifo			// 队列实现
├── emb				// 业务类代码


###########V1.0.0 版本内容更新
1. 平台第1版本
