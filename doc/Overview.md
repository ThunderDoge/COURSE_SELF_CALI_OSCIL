#Overview

本工程是 主要使用STM32F407ZG芯片以及片上自带的ADC 完成的自校准电能表工程。

已经完成GUI、基本测量和自校准功能。（2021-11-19）

- 使用 FREERTOS进行任务调度
- 已配置好的SEGGER SystemView支持
- 使用片上12位ADC进行采样。每一次采样需要15个ADC周期
- 使用按键和板上LCD屏幕交互的GUI
- 使用以太网接口进行通信。（但是未完成）

## 使用的开发工具

- STM32CubeMX 6.3.0 （代码生成器）
- Keil 5.28 （编译器/调试器）
- VSCode （辅助coding）
- SEGGER SystemView （操作系统性能监视器）



## 各任务线程

除非特别说明，每个任务都会开一个独立的线程进行工作。

所有的线程的业务代码位于`usertask.c`

1. ADC采样控制`ADCHandleTaskFunction`
   使用ADC1-Channel6
   - 启动由TIM2触发的采样序列。`Start_TIM_tiggered_ADC_DMA()`
   - Cube配置为Continueous DMA Request. 如此每次采样完成都会触发DMA传输。
   - DMA传输完指定数量之后会触发`HAL_ADC_ConvCpltCallback()`，在这里通知处理线程`ADCHandleTask`进行处理。
     :exclamation: 不要尝试在回调函数中进行处理，因为这等于在中断中进行数据处理。
   - `ADCHandleTask`将处理数据，根据数据调整下次采样的频率和偏移并且重新发起采样序列。
   - :grey_exclamation:由于FreeRTOS的调度频率是1000Hz所以数据处理迭代速度不可能超过1000Hz
2. DAC生成指令 `DACHandleTaskFunction`
   使用 DAC-Channel1
   两个功能：
   1. 在外部调用`HAL_DAC_SetValue()`, `HAL_DAC_Start()`设置值。
   2. 设置一个buffer按里面的内容持续的进行转唤。
3. UI管理
   使用了2个线程：`UIHandleTaskFunction`、`UI2ndDataDisplayTaskFunction`
   1. `UIHandleTaskFunction`中的`LcdMenu()`是菜单操作的总函数。
      进入下一级菜单即进入下一级函数。返回上一级菜单即返回函数。
   2. 菜单会通过写一些全局变量来给`ADCHandleTask`发消息来控制它的运作。
   3. `UI2ndDataDisplayTaskFunction`是显示参数的线程。
      - 这里显示部分使用了操作系统的Critical功能以避免LCD操作冲突。

## 主要功能模块

- `adc_control.c` ADC控制逻辑
  - 为ADC数据使用了双缓冲区`adc_buffer_0[]`, `adc_buffer_1[]`
  - 波形数据统计逻辑：`WaveformDataAnalyze()`
  - 使用环形数组队列做了滑块滤波：`WaveformSlidingBuffer wave_buffer`
    - 初始化：`InitializeWaveformSlidingBuffer()`
    - 填充队列：`FeedRegularSlidingBuffer()`
    - 取出数据到指定的结构体：`GetRegularSlidingOutput()`
  - 使用和上面同样的数据结构进行的校准过程
    - 初始化：`RequestCalibration()`
    - 填充队列：`FeedCalibration()`
    - 取出数据到指定的结构体：`GetCalibration()`
    - 取消校准：`ResetCalibration()`

- `gui.c` gui逻辑
  - `LcdMenu()` GUI 菜单
  - `LcdLcdDisplayParam()` 显示数据

## GUI操作指南

- MANUAL CONFIG/
  - VERTICAL/ 增益（量程）选择
    - 10V, 1V, 2V, 5V
    - 按下 ENTER键即应用
  - HORIZONTAL/ 采样频率选择
    - 1MSample/s ~ 1kSampl/s
    - 按下 ENTER键即应用
  - CALIBRATION/  校准
    :exclamation:在校准前请设定到合适的增益档位和频率档位
    - DC OFFSET CALI
    - GAIN OFFSET CALI
    - 进入并且按ENTER以开始校准
  - CONNECTION/ 网络连接（未实现）

