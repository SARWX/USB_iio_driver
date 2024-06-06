#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define CMD_GET_STATE 0x11
#define CMD_SET_STATE 0x22
#define CMD_GET_ADC_VAL 0x55

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A driver for my simple AMTEGA I2C ADC");

struct my_adc {						// Зададим структуру для нашего АЦП
	struct i2c_client *client;		// Ну, нам только I2C нужен
};

/** 
 * @brief This function will be caled every time 
 * when we want to read data from our sensor
 */
static int my_adc_read_raw(struct iio_dev * indio_dev, struct iio_chan_spec const * chan, int *val, int *val2, long mask) {
	struct my_adc *adc = iio_priv(indio_dev);		// каждый раз зачем-то создаем и связываем adc с indio_dev
	int ret;

	if(mask == IIO_CHAN_INFO_RAW) {										// Если запрос получить данные
		ret = i2c_smbus_read_byte_data(adc->client, CMD_GET_ADC_VAL);	// То читаем через I2C по адресу CMD_GET_ADC_VAL
		if(ret < 0) {
			printk("dt_iio - Error reading ADC value!\n");
			return ret;
		}
		*val = ret;		// записываем полученное значение в val
	}
	else 
		return -EINVAL;		// Сообщение об ошибке
	return IIO_VAL_INT;		// Сообщение об успехе
}

/** 
 * @brief This is channel structrure
 */
static const struct iio_chan_spec my_adc_channels[] = {
	{
		.type = IIO_VOLTAGE,								// По нашему каналу будут передаваться значения вольтов
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		// Данные не надо объединять, канал возвращает raw данные
	}
};

static const struct iio_info my_adc_info = {			// Вот в этой структуре мы задаем интерфейс к нашему indio_dev
	.read_raw = my_adc_read_raw,						// функция для чтения сырых данных
};

/* Declate the probe and remove functions */
static int my_adc_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int my_adc_remove(struct i2c_client *client);

static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "icv,myadc",			// Сообщаем, что этот драйвер, файл подходит для  "icv,myadc"
	}, { /* sentinel */ }					// Обозначение конца массива
};
MODULE_DEVICE_TABLE(of, my_driver_ids);		// Макрос, который доносит информацию до самого ядра Linux

static struct i2c_device_id my_adc[] = {	  // структура, которая используется для сопоставления I2C устройств с драйвером на основе имени устройства
	{"my_adc", 0},							  // имя устройства "my_adc" и ассоциированный с ним идентификатор, который в данном случае равен 0
	{ },									  // Обозначение конца массива
};
MODULE_DEVICE_TABLE(i2c, my_adc);

static struct i2c_driver my_driver = {
	.probe = my_adc_probe,					// будет вызвана, когда соответствующее устройство найдено
	.remove = my_adc_remove,				// будет вызвана, когда устройство удаляется
	.id_table = my_adc,						// таблица идентификаторов I2C устройств, с которыми этот драйвер совместим
	.driver = {								// Вложенная структура device_driver, описывающая основные параметры драйвера	
		.name = "my_adc",					// Имя драйвера
		.of_match_table = my_driver_ids,	// Указывает на таблицу совместимых устройств из Device Tree
	},
};

/**
 * @brief This function is called on loading the driver 
 */
static int my_adc_probe(struct i2c_client *client, const struct i2c_device_id *id) {
	struct iio_dev *indio_dev;				// Вот мы создали Industrial I/O device
	struct my_adc *adc;						// Вот наше I2C устройство 
	int ret;

	printk("dt_iio - Now I am in the Probe function!\n");

	if(client->addr != 0x10) {				// Сверяемся с нужным адресом
		printk("dt_iio - Wrong I2C address!\n");
		return -1;
	}

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(struct iio_dev));	// выделяем место под нашу iio_dev структуру
	// Но это не правильно, вот из документации:
	// struct iio_dev * devm_iio_device_alloc(struct device * dev, int sizeof_priv)
	// Типо dev - ок, но размер priv должен быть sizeof(struct my_adc)
	// я снизу напишу правильно, но пока закомментирую
	// indio_dev = devm_iio_device_alloc(&client->dev, sizeof(struct my_adc));
	if(!indio_dev) {
		printk("dt_iio - Error! Out of memory\n");
		return -ENOMEM;
	}

	adc = iio_priv(indio_dev);				// Вот тут мы связываем priv нашего device и кастомную структуру my_adc, точнее ее экземпляр adc 
	adc->client = client;					// А здесь связываем объект my_adc с i2c

	indio_dev->name = id->name;				// Ну тут мы просто заполняем структуру iio_dev 
	indio_dev->info = &my_adc_info;			// Напомню, iio_dev - главная структура в IIO
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = my_adc_channels;
	indio_dev->num_channels = ARRAY_SIZE(my_adc_channels);

	/* Set the ADC to power up mode */
	ret = i2c_smbus_write_byte_data(adc->client, CMD_SET_STATE, 0x1);		// Вот, здесь мы включаем наше устройство по I2C
	if(ret < 0) {
		printk("dt_iio - Error! Could not power ADC up\n");					// НЕ БУДЕТ РАБОТАТЬ, ЕСЛИ НЕТ ACK
		return -1;
	}

	/* Not shown in video, but useful! */
	/* Save indio_dev as i2c device data */
	i2c_set_clientdata(client, indio_dev);

	return devm_iio_device_register(&client->dev, indio_dev);				// Ну и регистрируем наше новое устройство в системе
																			// Здесь &client->dev - это реальное устройство
																			// А indio_dev - это структура Industrial I/O device
}

/**
 * @brief This function is called on unloading the driver 
 */
static int my_adc_remove(struct i2c_client *client) {
	struct iio_dev * indio_dev;
	struct my_adc *adc;

	printk("dt_iio - Now I am in the Remove function!\n");
	/* Not shown in video, but useful! */
	/* Set device to power down mode */
	indio_dev = i2c_get_clientdata(client);
	adc = iio_priv(indio_dev);
	i2c_smbus_write_byte_data(adc->client, CMD_SET_STATE, 0x0);

	return 0;
}

/* This will create the init and exit function automatically */
module_i2c_driver(my_driver);

