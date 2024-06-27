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
#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/triggered_buffer.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/errno.h>




#define VENDOR_ID 0x0483        // STMicroelectronics 
#define PRODUCT_ID 0xf125       // 
#define IIO_BUFFER_SIZE 128     // Размер пакчи изммерений, которая передаётся по USB

static int usb_cdc_iio_read_raw(struct iio_dev *indio_dev,
                                struct iio_chan_spec const *chan,
                                int *val,
                                int *val2,
                                long mask);
                                


// static int usb_cdc_iio_read_raw_multi(struct iio_dev *indio_dev,
//                                       struct iio_chan_spec const *chan,
//                                       int max_len,
//                                       int *vals,
//                                       int *val_len,
//                                       long mask);

// /**
//  * @brief Структура, описывающая драйвер
//  */
static const struct iio_info usb_cdc_iio_info = {			// Вот в этой структуре мы задаем интерфейс к нашему indio_dev
	.read_raw = usb_cdc_iio_read_raw,						// функция для чтения сырых данных
//    .read_raw_multi = usb_cdc_iio_read_raw_multi,           // функция для чтения всей пачки измерений
};

// Объявление обработчика прерывания USB
static irqreturn_t usb_cdc_iio_trigger_handler(int irq, void *p);

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

// /**
//  * @brief Структура для инициализации буфера
//  */
// static const struct iio_buffer_setup_ops usb_cdc_iio_buffer_setup_ops = {
//     .preenable = &iio_triggered_buffer_postenable,
//     .postdisable = &iio_triggered_buffer_predisable,
// };

// /**
//  * @brief инициализация
//  */
// static int usb_cdc_iio_buffer_init(struct iio_dev *indio_dev)
// {
//     int ret;

//     ret = iio_triggered_buffer_setup(indio_dev, NULL, &usb_cdc_iio_trigger_handler, &usb_cdc_iio_buffer_setup_ops);
//     if (ret) {
//         dev_err(&indio_dev->dev, "Failed to setup buffer\n");
//         return ret;
//     }

//     return 0;
// }

// /**
//  * @brief Удаление буфера
//  */
// static void usb_cdc_iio_buffer_cleanup(struct iio_dev *indio_dev)
// {
//     iio_triggered_buffer_cleanup(indio_dev);
// }



/**
 * @brief Наша собственная структура, описывающая устройство, для
 * которого мы разрабатываем драйвер
 */
struct usb_cdc_iio_dev {
    struct usb_device *udev;
    struct iio_dev *indio_dev;
    struct iio_trigger *trig;
    char buffer[64]; // Пример объявления буфера размером 64 байта, замените на нужный вам размер
    struct urb *urb;
};


static void usb_cdc_iio_urb_complete(struct urb *urb);
static void usb_cdc_iio_start_continuous_read(struct usb_cdc_iio_dev *dev);
static void usb_cdc_iio_stop_continuous_read(struct usb_cdc_iio_dev *dev);


/**
 * @brief Функция для управления триггером
 */
static int usb_cdc_iio_set_trigger_state(struct iio_trigger *trig, bool enable)
{
    struct usb_cdc_iio_dev *dev = iio_trigger_get_drvdata(trig);

    if (enable) {
        // Начинаем непрерывное чтение данных по USB
        usb_cdc_iio_start_continuous_read(dev); 
    } else {
        // Останавливаем непрерывное чтение данных по USB
        usb_cdc_iio_stop_continuous_read(dev);
    }

    return 0;
}


/**
 * @brief Структура опций трггера
 */
static const struct iio_trigger_ops usb_cdc_iio_trigger_ops = {
    .set_trigger_state = usb_cdc_iio_set_trigger_state,
    .validate_device = iio_trigger_validate_own_device,
};





/**
 * @brief Обработчик прерывания
 */
static irqreturn_t usb_cdc_iio_irq_handler(int irq, void *private)
{
    struct iio_trigger *trig = private;
    iio_trigger_poll(trig);
    return IRQ_HANDLED;
}


static irqreturn_t usb_cdc_iio_trigger_handler(int irq, void *p)
{
    struct iio_poll_func *pf = p;
    struct iio_dev *indio_dev = pf->indio_dev;
    struct usb_cdc_iio_dev *dev = iio_priv(indio_dev);
    int ret, actual_length;
    uint8_t buffer[16];

    ret = usb_bulk_msg(dev->udev,
                       usb_rcvbulkpipe(dev->udev, 1),
                       buffer,
                       sizeof(buffer),
                       &actual_length,
                       1000); // Таймаут в миллисекундах
    
    if (ret) {
        dev_err(&indio_dev->dev, "usb_bulk_msg failed: %d\n", ret);
        goto done;
    }

    if (actual_length > sizeof(buffer)) {
        dev_err(&indio_dev->dev, "Received too much data\n");
        ret = -EIO;
        goto done;
    }

    iio_push_to_buffers_with_timestamp(indio_dev, buffer, iio_get_time_ns(indio_dev));

done:
    iio_trigger_notify_done(indio_dev->trig);

    return IRQ_HANDLED;
}












//////////////////////// Блок работы с USB //////////////////////////
// Завершение URB
static void usb_cdc_iio_urb_complete(struct urb *urb)
{
    struct usb_cdc_iio_dev *dev = urb->context;

    if (urb->status) {
        printk(KERN_ERR "URB completion error: %d\n", urb->status);
        return;
    }

    // Обработка данных из dev->buffer
    // Например: iio_push_to_buffers_with_timestamp(dev->indio_dev, dev->buffer, iio_get_time_ns());

    // Перезапуск URB для непрерывного чтения данных
    usb_submit_urb(urb, GFP_ATOMIC);
}

// Запуск непрерывного чтения данных по USB
static void usb_cdc_iio_start_continuous_read(struct usb_cdc_iio_dev *dev) 
{
    int ret;

    dev->urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!dev->urb) {
        printk(KERN_ERR "Failed to allocate URB\n");
        return;
    }

    // dev->buffer = kzalloc(64, GFP_KERNEL); // Предполагаемый размер буфера  
    // if (!dev->buffer) {
    //     usb_free_urb(dev->urb);
    //     printk(KERN_ERR "Failed to allocate buffer\n");
    //     return;
    // }

    usb_fill_bulk_urb(dev->urb, dev->udev, usb_rcvbulkpipe(dev->udev, 1),
                      dev->buffer, 64, usb_cdc_iio_urb_complete, dev);

    ret = usb_submit_urb(dev->urb, GFP_KERNEL);
    if (ret) {
        kfree(dev->buffer);
        usb_free_urb(dev->urb);
        printk(KERN_ERR "Failed to submit URB: %d\n", ret);
    }
}

// Остановка непрерывного чтения данных по USB
static void usb_cdc_iio_stop_continuous_read(struct usb_cdc_iio_dev *dev)
{
    usb_kill_urb(dev->urb);
    kfree(dev->buffer);
    usb_free_urb(dev->urb);
}
/////////////////////////////////////////////////////////////////////
















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
    indio_dev = devm_iio_device_alloc(&interface->dev, sizeof(struct usb_cdc_iio_dev));     // Managed iio_device_alloc. iio_dev allocated with this function is automatically freed on driver detach
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

    // Настроим тригер
 //   struct iio_trigger *usb_cdc_iio_trig = 0;
    dev->trig = devm_iio_trigger_alloc(&interface->dev, indio_dev->name);       // Выделим память под триггер        TODO: в имени форматную строку, типа "%s-dev%d", indio_dev->name, iio_device_id(indio_dev)
                                                                                    //                                      чтобы можно было несколько одинаковых устройств использовать
    if (!dev->trig) 
    {
        printk(KERN_INFO "Unsucessful allocation triger ERROR\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "Sucessful allocation triger\n");
    // Добавим опции триггеру:
    dev->trig->ops = &usb_cdc_iio_trigger_ops;



    iio_trigger_set_drvdata(dev->trig, dev);

		/*
		 * The device generates interrupts as long as it is powered up.
		 * Some platforms might not allow the option to power it down so
		 * don't enable the interrupt to avoid extra load on the system
		 */
		ret = devm_request_irq(&interface->dev, interface_to_usbdev(interface)->irq, usb_cdc_iio_irq_handler,
                                IRQF_TRIGGER_FALLING | IRQF_NO_AUTOEN,
                                dev_name(&interface->dev), dev->trig);
        if (ret < 0) {
            printk(KERN_ERR "devm_request_irq failed with error %d\n", ret);
            return ret;
        }

        ret = devm_iio_trigger_register(&interface->dev, dev->trig);
        if (ret) {
            printk(KERN_ERR "devm_iio_trigger_register failed with error %d\n", ret);
            return ret;
        }








    // Настройка буфера
    // ret = usb_cdc_iio_buffer_init(indio_dev);
    // if (ret) {
    //     iio_device_free(indio_dev);
    //     return ret;
    // }

    printk(KERN_INFO "We are going to register iio device\n");
    ret = devm_iio_device_register(&interface->dev, indio_dev);     // Managed iio_device_register. The IIO device registered with this function is automatically unregistered on driver detach

    if (ret) {
        printk(KERN_ERR "iio_device_register failed with error %d\n", ret);
        usb_set_intfdata(interface, NULL);
        usb_put_dev(dev->udev);
        return ret;
    }

    printk(KERN_INFO "iio_device_register successful\n");
    return 0;
}

// Тут проблема, не правильно ресурсы освобождаются
static void usb_cdc_iio_disconnect(struct usb_interface *interface)
{
    usb_cdc_iio_stop_continuous_read(dev);
    // struct iio_dev *indio_dev = usb_get_intfdata(interface);
    struct iio_dev *indio_dev = usb_get_intfdata(interface);

//    iio_device_unregister(indio_dev);
    // usb_cdc_iio_buffer_cleanup(indio_dev);
    usb_set_intfdata(interface, NULL); // Устанавливаем интерфейсные данные в NULL
 //   usb_put_dev(dev->udev);  // Уменьшаем счетчик ссылок на USB устройство
//    kfree(dev); // Освобождаем память, выделенную для структуры usb_cdc_iio_dev
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
    /**
     * @brief Builds a bulk urb (USB Request Block),
     * sends it off and waits for completion 
     * If successful, 0. Otherwise a negative error number. 
     * The number of actual bytes transferred will be 
     * stored in the actual_length parameter
     * @param usb_dev - pointer to the usb device to send the message to
     * @param pipe - endpoint “pipe” to send the message to
     * @param data - pointer to the data to send
     * @param len - length in bytes of the data to send
     * @param actual_length - pointer to a location to put the actual length transferred in bytes
     * @param timeout - time in msecs to wait for the message to complete before timing out (if 0 the wait is forever)
     */
        ret = usb_bulk_msg(dev->udev,
                           usb_rcvbulkpipe(dev->udev, 1),
                           dev->buffer,
                           sizeof(dev->buffer),
                           &actual_length,
                           1000); // Таймаут в миллисекундах
        
        if (ret) {
            return ret; // Если ошибка (не 0), то вернуть код ошибки
        }

        if (actual_length > sizeof(dev->buffer)) {
            return -EIO; // Возвращаем ошибку, если получено больше данных, чем ожидается
        }

        // Проверяем, что val имеет достаточный размер
        if (!val) {
            return -ENOMEM; // Недостаточно памяти
        }
        if (actual_length >= sizeof(uint8_t)) {
            *val = *((uint8_t *)dev->buffer); // Assuming dev->buffer holds integer data
        } else {
            return -EIO; // Data received doesn't match expected size
        }

        // // Копируем данные в val
        // for (int i = 0; i < actual_length; i++) {
        //     val[i] = dev->buffer[i];
        // }

        return IIO_VAL_INT;

    default:
        return -EINVAL;
    }
}


// static int usb_cdc_iio_read_raw_multi(struct iio_dev *indio_dev,
//                                       struct iio_chan_spec const *chan,
//                                       int max_len,
//                                       int *vals,
//                                       int *val_len,
//                                       long mask)
// {
//     struct usb_cdc_iio_dev *dev = iio_priv(indio_dev);
//     int ret, actual_length;

//     switch (mask) {
//     case IIO_CHAN_INFO_RAW:
//         /**
//          * @brief Builds a bulk urb (USB Request Block),
//          * sends it off and waits for completion 
//          * If successful, 0. Otherwise a negative error number. 
//          * The number of actual bytes transferred will be 
//          * stored in the actual_length parameter
//          * @param usb_dev - pointer to the usb device to send the message to
//          * @param pipe - endpoint “pipe” to send the message to
//          * @param data - pointer to the data to send
//          * @param len - length in bytes of the data to send
//          * @param actual_length - pointer to a location to put the actual length transferred in bytes
//          * @param timeout - time in msecs to wait for the message to complete before timing out (if 0 the wait is forever)
//          */
//         ret = usb_bulk_msg(dev->udev,
//                            usb_rcvbulkpipe(dev->udev, 1),
//                            dev->buffer,
//                            sizeof(dev->buffer),
//                            &actual_length,
//                            1000); // Таймаут в миллисекундах
        
//         if (ret) {
//             return ret;         // Если ошибка (не 0), то вернуть код ошибки
//         }

//         if (actual_length < sizeof(dev->buffer)) {
//             return -EIO;        // Возвращаем ошибку, если не получено ожидаемое количество данных
//         }

//         // Проверяем, что max_len позволяет сохранить все данные
//         if (actual_length > max_len) {
//             return -ENOMEM;     // Недостаточно памяти для хранения данных
//         }

//         // Копируем данные из буфера устройства в массив значений
//         for (int i = 0; i < actual_length; i++) {
//             vals[i] = dev->buffer[i];
//         }

//         // Устанавливаем длину возвращенных данных
//         *val_len = actual_length;

//         return IIO_VAL_INT_MULTIPLE;

//     default:
//         return -EINVAL;
//     }
// }






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
