# RangerLow driverG

-----


Основной функционал данной прошивки для stm32f103. это управление двумя драйверами TA6586 и обработкой двух энкодеров, от моторов. управление осуществляется по средствам UART.


## Структруа пакета

`Core/Src/main.c`                  <- Основной поток программы выполняется в этом файле
`Core/Tarantul/motor_controller.c` <- В этом файле реализована логика управления моторами
`Core/Tarantul/uart_protocol.c`    <- Отвечает за создание и подготовку пакетов для передачи в UART

`Core/Inc/main.h`                  <- Тут все константы




## Структура пакета для установки скорости и получения данных энкодера

[START_BYTE, CORE_ID, CMD, DATA]

### Разеснение

START_BYTE <- начало пакета

CORE_ID    <- ID устройства

CMD        <- команда действия

DATA       <- значение              


|CMD |НОМЕР| Описание|
|----|-----|---------|
|M1_PWM|0x0A|Заставляет мотор крутиться с указаной скоростью DATA|
|M2_PWM|0x0B|Заставляет мотор крутиться с указаной скоростью DATA|
|M1_ENC|0x1A|Возвращает значение энкодера M1|
|M2_ENC|0x1B|Возвращает значение энкодера M2|






