static mm_segment_t old_fs;

static int kernel_file_open(struct file **fp, const char *filename, int open_mode, int mode)
{
    if (filename == NULL || fp == NULL) {
        printk("%s: invalid arguments.\n", __func__);
        return -EFAULT;
    }

    *fp = filp_open(filename, open_mode, mode);
    if (IS_ERR(*fp)) {
        printk("open %s error.\n", filename);
        return -EFAULT;
    }

    return 0;
}

static loff_t kernel_file_size(const char *filename)
{
    struct file *fp = NULL;
    struct inode *inode = NULL;
    loff_t fsize = 0;
    int ret = -1;

    ret = kernel_file_open(&fp, filename, O_RDONLY, 0);
    if (ret) {
        return -EFAULT;
    }

    inode = fp->f_dentry->d_inode;
    fsize = inode->i_size;

    inode = NULL;
    filp_close(fp, NULL);

    return fsize;
}

static int kernel_addr_limit_expend(void)
{
    old_fs = get_fs();

    set_fs(KERNEL_DS);

    return 0;
}

static int kernel_addr_limit_resume(void)
{
    set_fs(old_fs);

    return 0;
}

static ssize_t kernel_file_read(const char *filename, const char *buff, int count, loff_t off)
{
    struct file *fp = NULL;
    size_t t = -1;
    int ret = -1;

    if (buff == NULL) {
        printk("%s: invalid buff.\n", __func__);
        return -EFAULT;
    }

    ret = kernel_file_open(&fp, filename, O_RDONLY, 0);
    if (ret) {
        return -EFAULT;
    }

    kernel_addr_limit_expend();
    t = vfs_read(fp, buff, count, &off);
    vfs_fsync(fp, 0);
    kernel_addr_limit_resume();
    filp_close(fp, NULL);

    return t;
}

static ssize_t kernel_file_write(const char *filename, int open_mode, int mode, const char *buff, int count, loff_t off)
{
    struct file *fp = NULL;
    size_t t = -1;
    int ret = -1;

    if (buff == NULL) {
        printk("%s: invalid buff.\n", __func__);
        return -EFAULT;
    }

    ret = kernel_file_open(&fp, filename, open_mode, mode);
    if (ret) {
        return -EFAULT;
    }

    kernel_addr_limit_expend();
    t = vfs_write(fp, buff, count, &off);
    vfs_fsync(fp, 0);
    kernel_addr_limit_resume();
    filp_close(fp, NULL);

    return t;
}
