/**
 * iio_push_to_buffers() - push to a registered buffer.
 * @indio_dev:		iio_dev structure for device.
 * @data:		Full scan.
 */
int iio_push_to_buffers(struct iio_dev *indio_dev, const void *data)
{
	struct iio_dev_opaque *iio_dev_opaque = to_iio_dev_opaque(indio_dev);
	int ret;
	struct iio_buffer *buf;

	list_for_each_entry(buf, &iio_dev_opaque->buffer_list, buffer_list) {
		ret = iio_push_to_buffer(buf, data);
		if (ret < 0)
			return ret;
	}

	return 0;
}


static int iio_push_to_buffer(struct iio_buffer *buffer, const void *data)
{
	const void *dataout = iio_demux(buffer, data);
	int ret;

	ret = buffer->access->store_to(buffer, dataout);
	if (ret)
		return ret;

	/*
	 * We can't just test for watermark to decide if we wake the poll queue
	 * because read may request less samples than the watermark.
	 */
	wake_up_interruptible_poll(&buffer->pollq, EPOLLIN | EPOLLRDNORM);
	return 0;
}


static const void *iio_demux(struct iio_buffer *buffer,
			     const void *datain)
{
	struct iio_demux_table *t;

	if (list_empty(&buffer->demux_list))
		return datain;
	list_for_each_entry(t, &buffer->demux_list, l)
		memcpy(buffer->demux_bounce + t->to,
		       datain + t->from, t->length);

	return buffer->demux_bounce;
}


/**
 * struct iio_buffer - general buffer structure
 *
 * Note that the internals of this structure should only be of interest to
 * those writing new buffer implementations.
 */
struct iio_buffer {
	/** @length: Number of datums in buffer. */
	unsigned int length;

	/** @flags: File ops flags including busy flag. */
	unsigned long flags;

	/**  @bytes_per_datum: Size of individual datum including timestamp. */
	size_t bytes_per_datum;

	/* @direction: Direction of the data stream (in/out). */
	enum iio_buffer_direction direction;

	/**
	 * @access: Buffer access functions associated with the
	 * implementation.
	 */
	const struct iio_buffer_access_funcs *access;

	/** @scan_mask: Bitmask used in masking scan mode elements. */
	long *scan_mask;

	/** @demux_list: List of operations required to demux the scan. */
	struct list_head demux_list;

	/** @pollq: Wait queue to allow for polling on the buffer. */
	wait_queue_head_t pollq;

	/** @watermark: Number of datums to wait for poll/read. */
	unsigned int watermark;

	/* private: */
	/* @scan_timestamp: Does the scan mode include a timestamp. */
	bool scan_timestamp;

	/* @buffer_attr_list: List of buffer attributes. */
	struct list_head buffer_attr_list;

	/*
	 * @buffer_group: Attributes of the new buffer group.
	 * Includes scan elements attributes.
	 */
	struct attribute_group buffer_group;

	/* @attrs: Standard attributes of the buffer. */
	const struct iio_dev_attr **attrs;

	/* @demux_bounce: Buffer for doing gather from incoming scan. */
	void *demux_bounce;

	/* @attached_entry: Entry in the devices list of buffers attached by the driver. */
	struct list_head attached_entry;

	/* @buffer_list: Entry in the devices list of current buffers. */
	struct list_head buffer_list;

	/* @ref: Reference count of the buffer. */
	struct kref ref;

	/* @dmabufs: List of DMABUF attachments */
	struct list_head dmabufs; /* P: dmabufs_mutex */

	/* @dmabufs_mutex: Protects dmabufs */
	struct mutex dmabufs_mutex;
};



struct iio_buffer_access_funcs {
	int (*store_to)(struct iio_buffer *buffer, const void *data);
	int (*read)(struct iio_buffer *buffer, size_t n, char __user *buf);
	size_t (*data_available)(struct iio_buffer *buffer);
	int (*remove_from)(struct iio_buffer *buffer, void *data);
	int (*write)(struct iio_buffer *buffer, size_t n, const char __user *buf);
	size_t (*space_available)(struct iio_buffer *buffer);

	int (*request_update)(struct iio_buffer *buffer);

	int (*set_bytes_per_datum)(struct iio_buffer *buffer, size_t bpd);
	int (*set_length)(struct iio_buffer *buffer, unsigned int length);

	int (*enable)(struct iio_buffer *buffer, struct iio_dev *indio_dev);
	int (*disable)(struct iio_buffer *buffer, struct iio_dev *indio_dev);

	void (*release)(struct iio_buffer *buffer);

	struct iio_dma_buffer_block * (*attach_dmabuf)(struct iio_buffer *buffer,
						       struct dma_buf_attachment *attach);
	void (*detach_dmabuf)(struct iio_buffer *buffer,
			      struct iio_dma_buffer_block *block);
	int (*enqueue_dmabuf)(struct iio_buffer *buffer,
			      struct iio_dma_buffer_block *block,
			      struct dma_fence *fence, struct sg_table *sgt,
			      size_t size, bool cyclic);
	void (*lock_queue)(struct iio_buffer *buffer);
	void (*unlock_queue)(struct iio_buffer *buffer);

	unsigned int modes;
	unsigned int flags;
};










static int iio_store_to_kfifo(struct iio_buffer *r,
			      const void *data)
{
	int ret;
	struct iio_kfifo *kf = iio_to_kfifo(r);
	ret = kfifo_in(&kf->kf, data, 1);
	if (ret != 1)
		return -EBUSY;
	return 0;
}


unsigned int __kfifo_in(struct __kfifo *fifo,
		const void *buf, unsigned int len)
{
	unsigned int l;

	l = kfifo_unused(fifo);
	if (len > l)
		len = l;

	kfifo_copy_in(fifo, buf, len, fifo->in);
	fifo->in += len;
	return len;
}



// CUSTOM
static int iio_store_to_kfifo(struct iio_buffer *r,
			      const void *data)
{
//	int ret;
	struct iio_kfifo *kf = iio_to_kfifo(indio_dev.buffer);      // Если что iio_to_kfifo - это не функция, а макрос, он не должен повлиять на производительность
	kfifo_in(&kf->kf, data, IIO_BUFFER_SIZE);
	// if (ret != 1)
	// 	return -EBUSY;
	// return 0;
}
