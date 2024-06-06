#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define VENDOR_ID 0x0483        // STMicroelectronics 
#define PRODUCT_ID 0xf125       // 

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
    struct iio_dev *indio_dev;      // ЗАЧЕМ????
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

    indio_dev = iio_device_alloc(sizeof(*dev));
    if (!indio_dev)
        return -ENOMEM;

    dev = iio_priv(indio_dev);          // Достаточно забавно, т.е. место-то мы вделили,
                                        // Но связать указатель с местом надо вручную
    dev->udev = usb_get_dev(interface_to_usbdev(interface));        // interface_to_usbdev() преобразует указатель на интерфейс в указатель на устрорйство
                                                                    // Устройство может иметь несоклько интерфейсов
                                                                    // usb_get_dev() увеличивает счетчик ссылок на устройство в его структуре
                                                                    // (это нужно для того, чтобы понять когда можно освобождать ресурсы)
    dev->indio_dev = indio_dev;                                     // Связываем наше устройство с IIO слоем

    indio_dev->name = "usb_cdc_iio";            // Как будет отображаться в логах и системе
    indio_dev->info = &usb_cdc_iio_info;        // Эта структура определяет, как ядро будет взаимодействовать с устройством для выполнения операций чтения, записи и других
    indio_dev->modes = INDIO_DIRECT_MODE;       // устройство поддерживает прямой доступ к данным без необходимости буферизации или других сложных режимов работы

    usb_set_intfdata(interface, indio_dev);     // Связывает интерфейс с нашим абстрактным IIO устройством

    return iio_device_register(indio_dev);      // Зарегистрируй IIO устройство 
}

static void usb_cdc_iio_disconnect(struct usb_interface *interface)
{
    struct iio_dev *indio_dev = usb_get_intfdata(interface);
    struct usb_cdc_iio_dev *dev = iio_priv(indio_dev);

    iio_device_unregister(indio_dev);
    usb_put_dev(dev->udev);             // антипод функции usb_get_dev, т.е. это функция декремента счетчика ссылок
    iio_device_free(indio_dev);
}

static struct usb_driver usb_cdc_iio_driver = { 
    .name = "usb_cdc_iio",                  // Задаем информацию о разработанном
    .id_table = usb_cdc_id_table,           // драйвере
    .probe = usb_cdc_iio_probe,             // указываем нужные функции
    .disconnect = usb_cdc_iio_disconnect,
};

module_usb_driver(usb_cdc_iio_driver);      // макрос регистрирует USB-драйвер в ядре Linux

MODULE_AUTHOR("Ваше Имя");                  // макросы добавляют метаданные к модулю, 
MODULE_DESCRIPTION("USB CDC IIO Driver");   // которые могут быть полезны для 
MODULE_LICENSE("GPL");                      // информации о модуле и лицензировании
