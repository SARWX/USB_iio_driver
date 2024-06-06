# Объявление переменной obj-m, которая указывает на объектный файл модуля ядра
obj-m += dt_iio.o

# Цель для сборки модуля ядра и Device Tree Overlay
all: module dt
	echo Builded Device Tree Overlay and kernel module 

# Цель для сборки модуля ядра
module:
# Компиляция модуля ядра с помощью встроенных правил make в директории ядра
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Цель для сборки Device Tree Overlay из файла .dts
dt: testoverlay.dts
# Компиляция файла .dts в .dtbo с помощью утилиты dtc (Device Tree Compiler)
	dtc -@ -I dts -O dtb -o testoverlay.dtbo testoverlay.dts

# Цель для очистки (удаления) скомпилированных файлов и объектов
clean:
# Очистка сборочного окружения для модуля ядра
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
# Удаление сгенерированного файла Device Tree Overlay
	rm -rf testoverlay.dtbo
