#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define VENDOR_ID 0x0483        // STMicroelectronics 
#define PRODUCT_ID 0xf125       // 

static int usb_cdc_iio_read_raw(struct iio_dev *indio_dev,
                                struct iio_chan_spec const *chan,
                                int *val,
                                int *val2,
                                long mask);

// /**
//  * @brief Структура, описывающая драйвер
//  */
static const struct iio_info usb_cdc_iio_info = {			// Вот в этой структуре мы задаем интерфейс к нашему indio_dev
	.read_raw = usb_cdc_iio_read_raw,						// функция для чтения сырых данных
};

/** 
 * @brief This is channel structrure
 */
static const struct iio_chan_spec usb_cdc_iio_channels[] = {
	{
		.type = IIO_VOLTAGE,								// По нашему каналу будут передаваться значения вольтов
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		// Данные не надо объединять, канал возвращает raw данные
	}
};

/**
 * @brief Эта таблица содержит VID и PID, для которых
 * предназначен наш драйвер, в нашем случае VID - STMicroelectronics
 */
static const struct usb_device_id usb_cdc_id_table[] = {
    { USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
    { }
};

/**
 * @brief Макрос, который сообщает ядру, что этот драйвер 
 * поддерживает устройства, указанные в массиве usb_cdc_id_table
 * в принципе макрос добавляет даннные в некую общую таблицу
*/
MODULE_DEVICE_TABLE(usb, usb_cdc_id_table);

/**
 * @brief Наша собственная структура, описывающая устройство, для
 * которого мы разрабатываем драйвер
*/
struct usb_cdc_iio_dev {
    struct usb_device *udev;
    struct iio_dev *indio_dev;
    char buffer[64]; // Пример объявления буфера размером 64 байта, замените на нужный вам размер
};


/**
 * @brief Функция вызывается, когда устройство подключено 
 * и совпадает с одним из идентификаторов в usb_cdc_id_table
 * В самой функции происходит инициализация устройства, а именно:
 *     Выделяет память для IIO устройства (iio_device_alloc).
 *     Инициализирует структуру usb_cdc_iio_dev с указателями на устройство USB и IIO.
 *     Настраивает имя устройства IIO и его режим работы.
 *     Устанавливает приватные данные интерфейса USB (usb_set_intfdata).
 *     Регистрирует IIO устройство (iio_device_register).
 * @param usb_interface - структура, описывающая конкретный USB интерфейс, 
 * там хранятся конечные точки, настройки и т.п.
 * @param id - запись из таблицы идентификаторов USB устройств (usb_device_id), 
 * которая совпала с идентификаторами подключенного устройства
*/
static int usb_cdc_iio_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usb_cdc_iio_dev *dev;
    struct iio_dev *indio_dev;
    int ret = 0;


    printk(KERN_INFO "Try to allocate mem for indio_dev\n");
    indio_dev = devm_iio_device_alloc(&interface->dev, sizeof(struct usb_cdc_iio_dev));
 //   indio_dev = iio_device_alloc(dev, sizeof(*dev));
    if (!indio_dev) 
    {
        printk(KERN_INFO "Unsucessful allocation indio_dev ERROR\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "Sucessful allocation indio_dev\n");

    dev = iio_priv(indio_dev);          // Достаточно забавно, т.е. место-то мы вделили,
                                        // Но связать указатель с местом надо вручную
    dev->udev = usb_get_dev(interface_to_usbdev(interface));        // interface_to_usbdev() преобразует указатель на интерфейс в указатель на устрорйство
                                                                    // Устройство может иметь несоклько интерфейсов
                                                                    // usb_get_dev() увеличивает счетчик ссылок на устройство в его структуре
                                                                    // (это нужно для того, чтобы понять когда можно освобождать ресурсы)
    dev->indio_dev = indio_dev;                                     // Связываем наше устройство с IIO слоем

    indio_dev->name = "usb_cdc_iio";            // Как будет отображаться в логах и системе
//  //   indio_dev->info = &usb_cdc_iio_info;        // Эта структура определяет, как ядро будет взаимодействовать с устройством для выполнения операций чтения, записи и других
//     indio_dev->modes = INDIO_DIRECT_MODE;       // устройство поддерживает прямой доступ к данным без необходимости буферизации или других сложных режимов работы


	indio_dev->name = "osciloscope";				// Ну тут мы просто заполняем структуру iio_dev 
	indio_dev->info = &usb_cdc_iio_info;			// Напомню, iio_dev - главная структура в IIO
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = usb_cdc_iio_channels;
	indio_dev->num_channels = ARRAY_SIZE(usb_cdc_iio_channels);

    usb_set_intfdata(interface, dev);
    printk(KERN_INFO "We are going to register iio device\n");
    ret = devm_iio_device_register(&interface->dev, indio_dev);
    if (ret) {
        printk(KERN_ERR "iio_device_register failed with error %d\n", ret);
        usb_set_intfdata(interface, NULL);
        usb_put_dev(dev->udev);
        return ret;
    }

    printk(KERN_INFO "iio_device_register successful\n");
    return 0;
}

static void usb_cdc_iio_disconnect(struct usb_interface *interface)
{
    struct iio_dev *indio_dev = usb_get_intfdata(interface);
    struct usb_cdc_iio_dev *dev = iio_priv(indio_dev);

    iio_device_unregister(indio_dev);
    usb_put_dev(dev->udev);  // Уменьшаем счетчик ссылок на USB устройство
    usb_set_intfdata(interface, NULL); // Устанавливаем интерфейсные данные в NULL
    kfree(dev); // Освобождаем память, выделенную для структуры usb_cdc_iio_dev
}



static int usb_cdc_iio_read_raw(struct iio_dev *indio_dev,
                                struct iio_chan_spec const *chan,
                                int *val,
                                int *val2,
                                long mask)
{
    struct usb_cdc_iio_dev *dev = iio_priv(indio_dev);
    int ret, actual_length;

    switch (mask) {
    case IIO_CHAN_INFO_RAW:
        ret = usb_bulk_msg(dev->udev,
                           usb_rcvbulkpipe(dev->udev, 1),
                           dev->buffer,
                           sizeof(dev->buffer),
                           &actual_length,
                           1000); // Таймаут в миллисекундах
        if (ret) {
            return ret;
        }

        if (actual_length < sizeof(dev->buffer)) {
            return -EIO; // Возвращаем ошибку, если не получено ожидаемое количество данных
        }

        *val = dev->buffer[0]; // Пример: возвращаем первый байт прочитанных данных
        return IIO_VAL_INT;

    default:
        return -EINVAL;
    }
}



static struct usb_driver usb_cdc_iio_driver = { 
    .name = "usb_cdc_iio",                  // Задаем информацию о разработанном
    .id_table = usb_cdc_id_table,           // драйвере
    .probe = usb_cdc_iio_probe,             // указываем нужные функции
    .disconnect = usb_cdc_iio_disconnect,
};

module_usb_driver(usb_cdc_iio_driver);      // макрос регистрирует USB-драйвер в ядре Linux

MODULE_AUTHOR("ICV");                  // макросы добавляют метаданные к модулю, 
MODULE_DESCRIPTION("USB CDC IIO Driver for osciloscope");   // которые могут быть полезны для 
MODULE_LICENSE("GPL");                      // информации о модуле и лицензировании