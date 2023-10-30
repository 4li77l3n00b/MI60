# MI60
High performance, GH60 form factor Hall Effect Keyboard

## dir
```
|----Core               #[cubeMX]*
|  |----
|  |  
|----Drivers            #STM32H750官方驱动 [cubeMX]
|
|----MI                 #MI键盘相关类
|
|----Middlewares        #中间件    [cubeMX]
|
|----USB_DEVICE         #[cubeMX]*
|
|----UserApp            #
```


## flashRW

total memory 16MB
|编号|作用|内存空间|
|---|---|---|
|0|霍尔校准数据||
|1|触发参数||
|2|触发配置||
|3|键位Map||
|4|RGB Map||
|5|RGB FX||