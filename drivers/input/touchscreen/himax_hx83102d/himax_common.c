/*******************************************************************************
** Copyright (C), 2008-2016, OPPO Mobile Comm Corp., Ltd.
** ODM_WT_EDIT
** FILE: - hiamx_common.c
** Description : This program is for hiamx driver
** Version: 1.0
** Date : 2018/5/22
**
** -------------------------Revision History:----------------------------------
**  <author>	 <data> 	<version >			<desc>
**
**
*******************************************************************************/

#include "himax_common.h"
#include "himax_ic_core.h"

#define SUPPORT_FINGER_DATA_CHECKSUM 0x0F
#define TS_WAKE_LOCK_TIMEOUT		(2 * HZ)
#define FRAME_COUNT 5
extern int hx_ctpmodule;
extern int g_gesture;

int himax_tp = 0;

#if defined(HX_AUTO_UPDATE_FW) || defined(HX_ZERO_FLASH)
char *i_CTPM_firmware_name = "himax_firmware_hlt_05.bin";
char *i_CTPM_firmware_name_TCL = "himax_firmware_tcl.bin";

char *oppo_CTPM_firmware_name = "vendor/firmware/himax_firmware_hlt_05.bin";
char *oppo_CTPM_firmware_name_TCL = "vendor/firmware/himax_firmware_tcl.bin";

bool g_auto_update_flag = false;
u8 fw_arr[HX64K];
bool fw_arr_load_ok = false;//wdd  maybe load fail
u8 fw_arr_sort[HX64K];
bool fw_arr_sort_load_ok = false;//wdd  maybe load fail

#endif

extern int himax_black_chip_self_test(void);
extern int himax_mcu_0f_operation_dirly(void);
void lcd_resume_load_himax_fw(void);

#if defined(HX_AUTO_UPDATE_FW)
unsigned char *i_CTPM_FW = NULL;
int i_CTPM_FW_len;
int g_i_FW_VER = 0;
int g_i_CFG_VER = 0;
int g_i_CID_MAJ = 0; /*GUEST ID*/
int g_i_CID_MIN = 0; /*VER for GUEST*/
#endif
#ifdef ODM_WT_EDIT
#ifdef HX_ZERO_FLASH
int g_f_0f_updat = 0;
#endif
#endif
struct himax_ts_data *private_ts;
struct himax_ic_data *ic_data;
struct himax_report_data *hx_touch_data;
struct himax_core_fp g_core_fp;
struct himax_debug *debug_data;
extern struct fw_operation *pfw_op;
#ifdef HX_EDGE_LIMIT
struct corner_info corner_info[4];
struct point_info points_info[10];
#endif


struct proc_dir_entry *himax_touch_proc_dir;
#define HIMAX_PROC_TOUCH_FOLDER 	"touchpanel"
#ifdef CONFIG_TOUCHSCREEN_HIMAX_DEBUG
	extern int himax_debug_init(void);
	extern int himax_debug_remove(void);
#endif
/*ts_work about start*/
struct himax_target_report_data *g_target_report_data = NULL;
int himax_report_data(struct himax_ts_data *ts, int ts_path, int ts_status);
void himax_report_all_leave_event(struct himax_ts_data *ts);
/*ts_work about end*/
static int		HX_TOUCH_INFO_POINT_CNT;

unsigned long	FW_VER_MAJ_FLASH_ADDR;
unsigned long 	FW_VER_MIN_FLASH_ADDR;
unsigned long 	CFG_VER_MAJ_FLASH_ADDR;
unsigned long 	CFG_VER_MIN_FLASH_ADDR;
unsigned long 	CID_VER_MAJ_FLASH_ADDR;
unsigned long 	CID_VER_MIN_FLASH_ADDR;
/*unsigned long	PANEL_VERSION_ADDR;*/

unsigned long 	FW_VER_MAJ_FLASH_LENG;
unsigned long 	FW_VER_MIN_FLASH_LENG;
unsigned long 	CFG_VER_MAJ_FLASH_LENG;
unsigned long 	CFG_VER_MIN_FLASH_LENG;
unsigned long 	CID_VER_MAJ_FLASH_LENG;
unsigned long 	CID_VER_MIN_FLASH_LENG;
/*unsigned long	PANEL_VERSION_LENG;*/

unsigned long 	FW_CFG_VER_FLASH_ADDR;

unsigned char	IC_CHECKSUM = 0;
//extern int  HX_power_on(struct himax_ts_data *ts, bool on);
//extern int himax_power_init(struct himax_ts_data *ts, bool on);
#ifdef HX_ESD_RECOVERY
	u8 HX_ESD_RESET_ACTIVATE = 0;
	int hx_EB_event_flag = 0;
	int hx_EC_event_flag = 0;
	int hx_ED_event_flag = 0;
	int g_zero_event_count = 0;
#endif
u8 	HX_HW_RESET_ACTIVATE = 0;

#if defined(HX_PLATFOME_DEFINE_KEY)
	extern void	himax_platform_key(void);
#endif

extern int himax_parse_dt(struct himax_ts_data *ts,
						struct himax_i2c_platform_data *pdata);

static uint8_t 	AA_press = 0x00;
static uint8_t 	EN_NoiseFilter = 0x00;
static uint8_t	Last_EN_NoiseFilter = 0x00;

static int	p_point_num	= 0xFFFF;
#if defined(HX_EN_SEL_BUTTON) || defined(HX_EN_MUT_BUTTON)
static uint8_t 	vk_press = 0x00;
static int	tpd_key	   	= 0x00;
static int	tpd_key_old	= 0x00;
#endif
static int	probe_fail_flag;
/*wdd
#ifdef HX_USB_DETECT_GLOBAL
	bool USB_detect_flag;
#endif
*/
#if defined(CONFIG_FB)
int fb_notifier_callback(struct notifier_block *self,
						unsigned long event, void *data);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
static void himax_ts_early_suspend(struct early_suspend *h);
static void himax_ts_late_resume(struct early_suspend *h);
#endif

#ifdef HX_GESTURE_TRACK
	static int gest_pt_cnt;
	static int gest_pt_x[GEST_PT_MAX_NUM];
	static int gest_pt_y[GEST_PT_MAX_NUM];
#if 0
	static int gest_start_x, gest_start_y, gest_end_x, gest_end_y;
	static int gest_width, gest_height, gest_mid_x, gest_mid_y;
#endif
	static int hx_gesture_coor[16];
#endif

int himax_report_data_init(void);

extern int himax_dev_set(struct himax_ts_data *ts);
extern int himax_input_register_device(struct input_dev *input_dev);

int g_ts_dbg = 0;

/* File node for  baselinetest , SMWP and HSEN - Start*/
	#define HIMAX_PROC_GAME_SWITCH_FILE	"game_switch_enable"
	struct proc_dir_entry *himax_proc_game_switch_en_file;
#ifdef HX_EDGE_LIMIT
	#define HIMAX_PROC_LIMIT_ENABLE_FILE	"oppo_tp_limit_enable"
	struct proc_dir_entry *himax_proc_limit_enable_file;
	#define HIMAX_PROC_LIMIT_AREA_FILE	"oppo_tp_limit_area"
	struct proc_dir_entry *himax_proc_limit_area_file;
	#define PAGESIZE 256
#endif

#define HIMAX_PROC_BASELINE_TEST_FILE	"baseline_test"
#define HIMAX_BLACKSCREEN_TEST	"black_screen_test"

struct proc_dir_entry *himax_proc_baseline_test_file;
struct proc_dir_entry *himax_blackscreen_test;


uint8_t HX_PROC_SEND_FLAG;
#ifdef HX_SMART_WAKEUP
	#define HIMAX_PROC_SMWP_FILE "SMWP"
	struct proc_dir_entry *himax_proc_SMWP_file = NULL;
	#define HIMAX_PROC_GESTURE_FILE "GESTURE"
	struct proc_dir_entry *himax_proc_coor_file = NULL;
	#define HIMAX_PROC_DOUBLE_TAP_EN_FILE	"double_tap_enable"
	struct proc_dir_entry *himax_proc_double_tap_en_file = NULL;
#ifdef HX_GESTURE_TRACK
	struct proc_dir_entry *himax_proc_GESTURE_file = NULL;
	#define HIMAX_PROC_COOR_FILE	"coordinate"
#endif
	uint8_t HX_SMWP_EN = 0;
#endif
#if defined(HX_SMART_WAKEUP) || defined(CONFIG_TOUCHSCREEN_HIMAX_INSPECT)
bool FAKE_POWER_KEY_SEND = true;
#endif

#ifdef HX_HIGH_SENSE
	#define HIMAX_PROC_HSEN_FILE "HSEN"
	struct proc_dir_entry *himax_proc_HSEN_file = NULL;
#endif

#if defined(HX_PALM_REPORT)
static int himax_palm_detect(uint8_t *buf)
{
	struct himax_ts_data *ts = private_ts;
	int32_t loop_i;
	int base = 0;
	int x = 0, y = 0, w = 0;

	loop_i = 0;
	base = loop_i * 4;
	x = buf[base] << 8 | buf[base + 1];
	y = (buf[base + 2] << 8 | buf[base + 3]);
	w = buf[(ts->nFinger_support * 4) + loop_i];
	I(" %s HX_PALM_REPORT_loopi=%d,base=%x,X=%x,Y=%x,W=%x \n", __func__, loop_i, base, x, y, w);
	if ((!atomic_read(&ts->suspend_mode)) && (x == 0xFA5A) && (y == 0xFA5A) && (w == 0x00))
		return PALM_REPORT;
	else
		return NOT_REPORT;
}
#endif

/* Game Switch Enable Start*/
static ssize_t hx_game_switch_write(struct file	*file, const char	__user *buffer,	size_t count,	loff_t *ppos)
{
	char buf[8]	=	{0};
	int	temp;
	bool ret;
	struct himax_ts_data *ts = private_ts;

	if (!ts)
			return count;

	if (count	>	3)
			count	=	3;
	if (copy_from_user(buf,	buffer,	count))	{
			E("%s: read	proc input error.\n",	__func__);
			return count;
	}

	sscanf(buf,	"%x",	&temp);

	if (temp !=	0) {
		I("%s, Touchpanel	game switch	enable %d\n",	__func__,	temp);
		ret	=	g_core_fp.fp_no_jiter_en(1);
			if (ret	== false)	{
					I("%s, Touchpanel	enable game	switch failed\n",	__func__);
			}
	}	else {
		I("%s, Touchpanel	game switch	disable	%d\n", __func__, temp);
		ret	=	g_core_fp.fp_no_jiter_en(0);
			if (ret	== false)	{
					I("%s, Touchpanel	disable	game switch	failed\n", __func__);
			}
	}

	return count;
}

static const struct	file_operations	hx_proc_game_switch_ops	= {
	.write = hx_game_switch_write,
	.open	 = simple_open,
	.owner = THIS_MODULE,
};
/* Game	Switch Enable	End*/

/* Edge	Limit	Start*/
#ifdef HX_EDGE_LIMIT
static ssize_t hx_limit_ctrl_read(struct file	*file, char	__user *user_buf,	size_t count,	loff_t *ppos)
{
	ssize_t	ret	=	0;
	char page[PAGESIZE];
	struct himax_ts_data *ts = private_ts;

	if (!ts)
			return count;

	I("limit_enable	is:	0x%x,	ts->limit_edge = 0x%x, ts->limit_corner	=	0x%x\n", ts->limit_enable, ts->limit_edge, ts->limit_corner);
	ret	=	snprintf(page, PAGESIZE,	"%d\n",	ts->limit_enable);

	ret	=	simple_read_from_buffer(user_buf,	count, ppos, page, strlen(page));

	return ret;
}

static ssize_t hx_limit_ctrl_write(struct	file *file,	const	char __user	*buffer, size_t	count, loff_t	*ppos)
{
	char buf[8]	=	{0};
	int	temp;
	bool ret;
	struct himax_ts_data *ts = private_ts;

	if (!ts)
			return count;

	if (count	>	3)
			count	=	3;
	if (copy_from_user(buf,	buffer,	count))	{
			E("%s: read	proc input error.\n",	__func__);
			return count;
	}

	sscanf(buf,	"%x",	&temp);
	if (temp > 0x1F) {
			I("%s: temp	=	0x%x > 0x1F	\n", __func__, temp);
			return count;
	}

	ts->limit_enable = temp;
	ts->limit_edge = ts->limit_enable	&	1;		/*bit0 to	control	the	feature	of edge	limit*/
	ts->limit_corner = ts->limit_enable	>> 1;	/*control	the	corner points	is release or	not*/

	I("%s: limit_enable	=	0x%x,	ts->limit_edge = 0x%x, ts->limit_corner=0x%x\n", __func__, ts->limit_enable, ts->limit_edge, ts->limit_corner);

	if (ts->suspended	== false)	{
		ret	=	g_core_fp.fp_edge_limit_en(ts->limit_edge);
			if (ret	== false)	{
					I("%s, Touchpanel	operate	mode switch	failed\n", __func__);
			}
	}

	return count;
}

static const struct	file_operations	hx_proc_limit_control_ops	= {
	.read	 = hx_limit_ctrl_read,
	.write = hx_limit_ctrl_write,
	.open	 = simple_open,
	.owner = THIS_MODULE,
};

static ssize_t proc_limit_area_read(struct file	*file, char	__user *user_buf,	size_t count,	loff_t *ppos)
{
	ssize_t	ret	=	0;
	char page[PAGESIZE];
	struct himax_ts_data *ts = private_ts;

	if (!ts)
		return count;
	I("limit_area	is:	%d\n", ts->edge_limit.limit_area);
	ret	=	snprintf(page, PAGESIZE/*wdd 18/07/25 over run PAGE_SIZE*/,	"limit_area	=	%d left_x1 = %d	right_x1 = %d	left_x2	=	%d right_x2	=	%d left_y1 = %d	right_y1 = %d	left_y2	=	%d right_y2	=	%d\n",
					ts->edge_limit.limit_area, ts->edge_limit.left_x1, ts->edge_limit.right_x1,	ts->edge_limit.left_x2,	ts->edge_limit.right_x2,
					ts->edge_limit.left_y1,	ts->edge_limit.right_y1, ts->edge_limit.left_y2, ts->edge_limit.right_y2);
	ret	=	simple_read_from_buffer(user_buf,	count, ppos, page, strlen(page));

	return ret;
}

static ssize_t proc_limit_area_write(struct	file *file,	const	char __user	*buffer, size_t	count, loff_t	*ppos)
{
	char buf[8];
	int	temp;
	struct himax_ts_data *ts = private_ts;

	if (!ts)
			return count;

	if (copy_from_user(buf,	buffer,	count))	{
			I("%s: read	proc input error.\n",	__func__);
			return count;
	}

	sscanf(buf,	"%d",	&temp);
	if (temp < 0 ||	temp > 10)
		return count;

	ts->edge_limit.limit_area	=	temp;
	ts->edge_limit.left_x1		=	(ts->edge_limit.limit_area*1000)/100;
	ts->edge_limit.right_x1		=	ic_data->HX_X_RES	-	ts->edge_limit.left_x1;
	ts->edge_limit.left_x2		=	2	*	ts->edge_limit.left_x1;
	ts->edge_limit.right_x2		=	ic_data->HX_X_RES	-	(2 * ts->edge_limit.left_x1);
	/*ts->edge_limit.right_x1		=	ic_data->HX_X_RES	-	ts->edge_limit.left_x1;
	ts->edge_limit.left_x2		=	2	*	ts->edge_limit.left_x1;
	ts->edge_limit.right_x2		=	ic_data->HX_X_RES	-	(2 * ts->edge_limit.left_x1);*/

	I("limit_area	=	%d;	left_x1	=	%d;	right_x1 = %d; left_x2 = %d; right_x2	=	%d\n",
				 ts->edge_limit.limit_area,	ts->edge_limit.left_x1,	ts->edge_limit.right_x1, ts->edge_limit.left_x2, ts->edge_limit.right_x2);

	ts->edge_limit.left_y1		=	(ts->edge_limit.limit_area*1000)/100;
	ts->edge_limit.right_y1		=	ts->edge_limit.left_y1;
	ts->edge_limit.left_y2		=	ic_data->HX_Y_RES	-	2	*	ts->edge_limit.left_y1;
	ts->edge_limit.right_y2		=	ic_data->HX_Y_RES	-	(2 * ts->edge_limit.left_y1);
	/*ts->edge_limit.right_y1		=	ic_data->HX_Y_RES	-	ts->edge_limit.left_y1;
	ts->edge_limit.left_y2		=	2	*	ts->edge_limit.left_y1;
	ts->edge_limit.right_y2		=	ic_data->HX_Y_RES	-	(2 * ts->edge_limit.left_y1);*/

	I("limit_area	=	%d;	left_y1	=	%d;	right_y1 = %d; left_y2 = %d; right_y2	=	%d\n",
					ts->edge_limit.limit_area, ts->edge_limit.left_y1, ts->edge_limit.right_y1,	ts->edge_limit.left_y2,	ts->edge_limit.right_y2);
	return count;
}

static const struct	file_operations	proc_limit_area_ops	= {
		.read	 = proc_limit_area_read,
		.write = proc_limit_area_write,
		.open	 = simple_open,
		.owner = THIS_MODULE,
};
#endif
/* Edge Limit End*/

static ssize_t himax_baseline_test_read(struct file *file, char *buf,
									size_t len, loff_t *pos)
{
	int val = 0x00;
	size_t ret = 0;
	char *temp_buf;
	I("%s: enter, %d \n", __func__, __LINE__);

	if ( *pos ) {
	    printk("is already read the file\n");
        return 0;
	}
    *pos += len;
		temp_buf = kzalloc(len, GFP_KERNEL);
		if (temp_buf == NULL) {
			printk("temp_buf = %p\n",temp_buf);
			return -1;
		}
		himax_int_enable(0);/* disable irq */
		//private_ts->in_baseline_test = 1;
		val = g_core_fp.fp_chip_baseline_test();
/*#ifdef HX_ESD_RECOVERY
		HX_ESD_RESET_ACTIVATE = 1;
#endif*/
		himax_int_enable(1);/* enable irq */

		if (val == pfw_op->data_selftest_pass[0]) {
			ret += snprintf(temp_buf + ret, len - ret, "0 error(s). All test passed.\n");
		} else {
			ret += snprintf(temp_buf + ret, len - ret, "error code=0x%02X, Test Fail!\n", val);
		}

		//private_ts->in_baseline_test = 0;

		if (copy_to_user(buf, temp_buf, ret))
			I("%s,here:%d\n", __func__, __LINE__);

		kfree(temp_buf);

	return ret;
}

static struct file_operations himax_proc_baseline_test_ops = {
	.owner = THIS_MODULE,
	.read = himax_baseline_test_read,
};

#ifdef HX_HIGH_SENSE
static ssize_t himax_HSEN_read(struct file *file, char *buf,
							   size_t len, loff_t *pos)
{
	struct himax_ts_data *ts = private_ts;
	size_t count = 0;
	char *temp_buf;

	if (!HX_PROC_SEND_FLAG) {
		temp_buf = kzalloc(len, GFP_KERNEL);
		count = snprintf(temp_buf, PAGE_SIZE, "%d\n", ts->HSEN_enable);

		if (copy_to_user(buf, temp_buf, len))
			I("%s,here:%d\n", __func__, __LINE__);

		kfree(temp_buf);
		HX_PROC_SEND_FLAG = 1;
	} else {
		HX_PROC_SEND_FLAG = 0;
	}

	return count;
}

static ssize_t himax_HSEN_write(struct file *file, const char *buff,
								size_t len, loff_t *pos)
{
	struct himax_ts_data *ts = private_ts;
	char buf[80] = {0};

	if (len >= 80) {
		I("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(buf, buff, len)) {
		return -EFAULT;
	}

	if (buf[0] == '0')
		ts->HSEN_enable = 0;
	else if (buf[0] == '1')
		ts->HSEN_enable = 1;
	else
		return -EINVAL;

	g_core_fp.fp_set_HSEN_enable(ts->HSEN_enable, ts->suspended);
	I("%s: HSEN_enable = %d.\n", __func__, ts->HSEN_enable);
	return len;
}

static struct file_operations himax_proc_HSEN_ops = {
	.owner = THIS_MODULE,
	.read = himax_HSEN_read,
	.write = himax_HSEN_write,
};
#endif

#ifdef HX_SMART_WAKEUP
/*oppo*/

#ifdef HX_GESTURE_TRACK

static void *himax_coor_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}
static void *himax_coor_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}
static void himax_coor_stop(struct seq_file *m, void *v)
{
	return;
}

static int32_t himax_coor_show(struct seq_file *m, void *v)
{
    int i = 0;
    //size_t ret = 0;
	char temp_buf[256] = {0};
    //uint16_t len = sizeof(temp_buf);

	I("%s enter.\n",__func__);//,len);

	#ifdef HX_GESTURE_TRACK

		for (i = 0; i < 8; i++) {
			if (i == 0) {
				//ret += snprintf(temp_buf + ret, len - ret, "%d,", hx_gesture_coor[i]);
				I("hx_gesture_coor[%d] = %d \n",i,hx_gesture_coor[i]);
			} else if (i == 7) {
				//ret += snprintf(temp_buf + ret, len - ret, "%d\n", hx_gesture_coor[i * 2 - 1]);
				I("hx_gesture_coor[%d] = %d \n",i * 2 - 1,hx_gesture_coor[i * 2 - 1]);
			} else {
				//ret += snprintf(temp_buf + ret, len - ret, "%d:", hx_gesture_coor[i * 2 - 1]);
				//ret += snprintf(temp_buf + ret, len - ret, "%d,", hx_gesture_coor[i * 2]);
				I("hx_gesture_coor[%d] = %d \n",i * 2 - 1,hx_gesture_coor[i * 2 - 1]);
				I("hx_gesture_coor[%d] = %d \n",i * 2,hx_gesture_coor[i * 2]);
			}
		}

		sprintf(temp_buf, "%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d",
			hx_gesture_coor[0],
			hx_gesture_coor[1], hx_gesture_coor[2],
			hx_gesture_coor[3], hx_gesture_coor[4],
			hx_gesture_coor[5], hx_gesture_coor[6],
			hx_gesture_coor[7], hx_gesture_coor[8],
			hx_gesture_coor[9], hx_gesture_coor[10],
			hx_gesture_coor[11], hx_gesture_coor[12],
			hx_gesture_coor[13]);

		/* oppo gesture formate */
		seq_printf(m, "%s\n", temp_buf);
	#endif
	I("%s end.\n",__func__);
	return 0;
}

const struct seq_operations oppo_coord_seq_ops =
{
    .start  = himax_coor_start,
    .next   = himax_coor_next,
    .stop   = himax_coor_stop,
    .show   = himax_coor_show,
};

static int32_t himax_diag_coor_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &oppo_coord_seq_ops);
}

static struct file_operations himax_proc_coor_ops = {
	.owner = THIS_MODULE,
	.open = himax_diag_coor_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};
#endif

static ssize_t himax_double_tap_en_write(struct file *file, const char *buff,/*double_tap_enable*/
								size_t len, loff_t *pos)
{
	struct himax_ts_data *ts = private_ts;
	char buf[80] = {0};
	int i = 0;
	/*if(ts->suspended) {
		I("%s: is already suspend\n", __func__);
		return -1;
	}*/

	if (len >= 80) {
		I("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(buf, buff, len)) {
		return -EFAULT;
	}
	
	if (buf[0] == '0') {
		ts->SMWP_enable = 0;
		g_gesture = 0;
		ts->gesture_cust_en[0] = 0;
	} else if (buf[0] == '1') {
		if (ts->SMWP_enable == 0 && ts->psensor_stus == 1) {
			mutex_lock(&private_ts->w_fw_lock);
			I("%s: Psensor far away +++++.\n", __func__);
			ts->SMWP_enable = 1;
			g_gesture = 1;
			ts->psensor_stus = 0;
			g_core_fp.fp_black_gest_en(true);
			g_core_fp.fp_set_SMWP_enable(ts->SMWP_enable, ts->suspended);
			mutex_unlock(&private_ts->w_fw_lock);
			return len;
		}
		ts->SMWP_enable = 1;
		g_gesture = 1;
		for (i = 0; i < 16; i++) {
			ts->gesture_cust_en[i] = 1;
		}
	}	else if (buf[0] == '2') {
			if (ts->SMWP_enable == 1 && ts->psensor_stus == 0) {
				mutex_lock(&private_ts->w_fw_lock);
				I("%s: Psensor near by ----.\n", __func__);
				ts->SMWP_enable = 0;
				ts->psensor_stus = 1;
				g_core_fp.fp_black_gest_en(false);
				mutex_unlock(&private_ts->w_fw_lock);
				return len;
			}
	}	else

		return -EINVAL;

	g_core_fp.fp_set_SMWP_enable(ts->SMWP_enable, ts->suspended);
	I("%s: SMART_WAKEUP_enable = %d.\n", __func__, ts->SMWP_enable);
	return len;
}

static ssize_t himax_double_tap_en_read(struct file *file, char *buf,
								  size_t len, loff_t *pos)
{
	size_t count = 0;
	struct himax_ts_data *ts = private_ts;
	char *temp_buf;

	if (!HX_PROC_SEND_FLAG) {
		temp_buf = kzalloc(len, GFP_KERNEL);
		count = snprintf(temp_buf, PAGE_SIZE, "%d\n", ts->SMWP_enable);

		if (copy_to_user(buf, temp_buf, len))
			I("%s,here:%d\n", __func__, __LINE__);

		kfree(temp_buf);
		HX_PROC_SEND_FLAG = 1;
	} else {
		HX_PROC_SEND_FLAG = 0;
	}

	return count;
}


static struct file_operations himax_proc_double_tap_en_ops = {
	.owner = THIS_MODULE,
	.read = himax_double_tap_en_read,
	.write = himax_double_tap_en_write,
};

static ssize_t himax_SMWP_read(struct file *file, char *buf,
							   size_t len, loff_t *pos)
{
	size_t count = 0;
	struct himax_ts_data *ts = private_ts;
	char *temp_buf;

	if (!HX_PROC_SEND_FLAG) {
		temp_buf = kzalloc(len, GFP_KERNEL);
		count = snprintf(temp_buf, PAGE_SIZE, "%d\n", ts->SMWP_enable);

		if (copy_to_user(buf, temp_buf, len))
			I("%s,here:%d\n", __func__, __LINE__);

		kfree(temp_buf);
		HX_PROC_SEND_FLAG = 1;
	} else {
		HX_PROC_SEND_FLAG = 0;
	}

	return count;
}

static ssize_t himax_SMWP_write(struct file *file, const char *buff,
								size_t len, loff_t *pos)
{
	struct himax_ts_data *ts = private_ts;
	char buf[80] = {0};

	if (len >= 80) {
		I("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(buf, buff, len)) {
		return -EFAULT;
	}

	if (buf[0] == '0')
		ts->SMWP_enable = 0;
	else if (buf[0] == '1')
		ts->SMWP_enable = 1;
	else
		return -EINVAL;

	g_core_fp.fp_set_SMWP_enable(ts->SMWP_enable, ts->suspended);
	HX_SMWP_EN = ts->SMWP_enable;
	I("%s: SMART_WAKEUP_enable = %d.\n", __func__, HX_SMWP_EN);
	return len;
}

static struct file_operations himax_proc_SMWP_ops = {
	.owner = THIS_MODULE,
	.read = himax_SMWP_read,
	.write = himax_SMWP_write,
};

static ssize_t himax_GESTURE_read(struct file *file, char *buf,
								  size_t len, loff_t *pos)
{
	struct himax_ts_data *ts = private_ts;
	int i = 0;
	size_t ret = 0;
	char *temp_buf;

	if (!HX_PROC_SEND_FLAG) {
		temp_buf = kzalloc(len, GFP_KERNEL);

		for (i = 0; i < 16; i++)
			ret += snprintf(temp_buf + ret, len - ret, "ges_en[%d]=%d \n", i, ts->gesture_cust_en[i]);

		if (copy_to_user(buf, temp_buf, len))
			I("%s,here:%d\n", __func__, __LINE__);

		kfree(temp_buf);
		HX_PROC_SEND_FLAG = 1;
	} else {
		HX_PROC_SEND_FLAG = 0;
		ret = 0;
	}

	return ret;
}

static ssize_t himax_GESTURE_write(struct file *file, const char *buff,
								   size_t len, loff_t *pos)
{
	struct himax_ts_data *ts = private_ts;
	int i = 0;
	char buf[80] = {0};

	if (len >= 80) {
		I("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(buf, buff, len)) {
		return -EFAULT;
	}

	I("himax_GESTURE_store= %s \n", buf);

	for (i = 0; i < 16; i++) {
		if (buf[i] == '0')
			ts->gesture_cust_en[i] = 0;
		else if (buf[i] == '1')
			ts->gesture_cust_en[i] = 1;
		else
			ts->gesture_cust_en[i] = 0;

		I("gesture en[%d]=%d \n", i, ts->gesture_cust_en[i]);
	}

	return len;
}

static struct file_operations himax_proc_Gesture_ops = {
	.owner = THIS_MODULE,
	.read = himax_GESTURE_read,
	.write = himax_GESTURE_write,
};
#endif
#ifdef ODM_WT_EDIT
//Tianchen.Zhao@ODM_RH.TP Porting
#ifdef CONFIG_TOUCHSCREEN_HIMAX_INSPECT
extern void (*fp_himax_baseline_test_init)(void);
#endif
#endif
//#endif
#ifdef ODM_WT_EDIT
//Tianchen.Zhao@ODM_RH.TP Porting
static ssize_t wt_hiamx_mptest_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
#if 0
	int ret = 0;
	int len = 0;
	size_t size = 0;
	char *ptr = NULL;
	char result[20] = "result=1";
	size = ARRAY_SIZE(result);
	if ( *ppos ) {
	    printk("is already read the file\n");
        return 0;
	}
    *ppos += count;
    //min = min(count,size);
    ptr = (char*)kzalloc(size,GFP_KERNEL);
    len = snprintf(ptr, size,"%s\n",result);
    ret = copy_to_user(buf,ptr,len);
	return len;
#endif
	int val = 0x00;
	size_t ret = 0;
	char *temp_buf;
	if ( *ppos ) {
	    printk("is already read the file\n");
        return 0;
	}
    *ppos += len;
	I("%s: enter, %d \n", __func__, __LINE__);


	temp_buf = kzalloc(len, GFP_KERNEL);
	if (temp_buf == NULL){
		printk("temp_buf = %p\n",temp_buf);
		return -1;
	}
		himax_int_enable(0);/* disable irq */
		//private_ts->in_baseline_test = 1;
		val = g_core_fp.fp_chip_baseline_test();
		private_ts->in_baseline_test = 0;
	  g_core_fp.fp_0f_operation_dirly();
		msleep(5);
		g_core_fp.fp_reload_disable(0);
		msleep(5);
		g_core_fp.fp_sense_on(0);
/*#ifdef HX_ESD_RECOVERY
		HX_ESD_RESET_ACTIVATE = 1;
#endif*/
		himax_int_enable(1);/* enable irq */

		if (val == pfw_op->data_selftest_pass[0]) {
			ret += snprintf(temp_buf + ret, len - ret, "result=1");
		} else {
			ret += snprintf(temp_buf + ret, len - ret, "error code=0x%02X, Test Fail!\n", val);
		}

		//private_ts->in_baseline_test = 0;

		if (copy_to_user(buf, temp_buf, ret))
			I("%s,here:%d\n", __func__, __LINE__);

		kfree(temp_buf);


	return ret;
}

static struct file_operations wt_himax_open_test_fops = {
	.read = wt_hiamx_mptest_read,
};
#endif

static ssize_t himax_blackscreen_test_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int val = 0x00;
    int ret = 0;
    int retry = 20;
	int len = 0;
	struct himax_ts_data *ts = private_ts;
	char *ptr = NULL;
    //NVT_LOG("%s %ld %lld\n", __func__, count, *ppos);
	if ( *ppos ) {
    	printk("is already read the file\n");
    	return 0;
	}
    *ppos += count;

    if (!ts->selftest_flag) {
		printk("ts->gesture_test.flag ==== %d\n",ts->selftest_flag);
		return 0;
	}
    ptr = kzalloc(count, GFP_KERNEL);
    if (ptr == NULL) {
        printk("failed to alloc ptr memory\n");
        return 0;
    }

    //wait until tp is in sleep, then sleep 500ms to make sure tp is in gesture mode
    do {
        if (ts->suspended == false) {
            msleep(500);
            break;
        }
        msleep(200);
    } while(--retry);

    printk("%s retry times %d\n", __func__, retry);
    if (retry == 0 && !ts->suspended) {
		len = snprintf(ptr,count,"%s\n", "1 errors: not in sleep");
        goto OUT;
    }
	himax_int_enable(0);/* disable irq */
	//private_ts->in_baseline_test = 1;
    //mutex_lock(&ts->lock);
	val = himax_black_chip_self_test();
/*#ifdef HX_ESD_RECOVERY
			HX_ESD_RESET_ACTIVATE = 1;
#endif*/

	himax_int_enable(1);/* disable irq */
	if (val == pfw_op->data_selftest_pass[0]) {
		len += snprintf(ptr + len, count - len, "0 error(s). All test passed.\n");
	} else {
		len += snprintf(ptr + len, count - len, "error Test Fail! 0x%02x\n",val);
	}
    //private_ts->in_baseline_test = 0;
OUT:
    ts->selftest_flag = 0;
	g_gesture = ts->gesture_bak;
    ts->SMWP_enable = ts->backup_flag;
	ret=copy_to_user(buf,ptr,len);
	kfree(ptr);
    return len;
}

static ssize_t himax_blackscreen_test_write(struct file *file, const char __user *userbuf, size_t count, loff_t *ppos)
{
    int value = 0;
	char buf[4] = {0};
	/*wdd 18/07/25 leak storage*/
	//char *ptr = NULL;
	struct himax_ts_data *ts = private_ts;
	/*
	ptr = kzalloc(count,GFP_KERNEL);
	if ( ptr == NULL ) {
		return -1;
	}
	*/
	if ( copy_from_user(buf, userbuf, count) ) {
		printk("%s: copy from user error.", __func__);
		return -1;
	}
	sscanf(buf, "%d", &value);

	printk("value ============%d\n",value);

    ts->backup_flag = ts->SMWP_enable;
	ts->gesture_bak = g_gesture;
    ts->SMWP_enable = 1;
	g_gesture = 1;
    ts->selftest_flag = value;

    return count;
}



static const struct file_operations himax_black_screen_test_fops = {
    .owner = THIS_MODULE,
    .read  = himax_blackscreen_test_read,
    .write = himax_blackscreen_test_write,
};

int himax_common_proc_init(void)
{
#ifdef ODM_WT_EDIT
//Tianchen.Zhao@ODM_RH.TP Porting
		struct proc_dir_entry *proc_mp_dir;
		struct proc_dir_entry *proc_mptest_node;
		proc_mp_dir = proc_mkdir("touchscreen", NULL);
		proc_mptest_node = proc_create("ctp_openshort_test", 0666, proc_mp_dir, &wt_himax_open_test_fops);
#endif

	himax_touch_proc_dir = proc_mkdir(HIMAX_PROC_TOUCH_FOLDER, NULL);

	if (himax_touch_proc_dir == NULL) {
		E(" %s: himax_touch_proc_dir file create failed!\n", __func__);
		return -ENOMEM;
	}
#ifdef CONFIG_TOUCHSCREEN_HIMAX_INSPECT
	if (fp_himax_baseline_test_init != NULL)
		fp_himax_baseline_test_init();
#endif

	himax_proc_baseline_test_file = proc_create(HIMAX_PROC_BASELINE_TEST_FILE, 0666, himax_touch_proc_dir, &himax_proc_baseline_test_ops);
	if (himax_proc_baseline_test_file == NULL) {
		E(" %s: proc baseline_test file create failed!\n", __func__);
		goto fail_oppo_1;
	}
	himax_blackscreen_test = proc_create(HIMAX_BLACKSCREEN_TEST,0666, himax_touch_proc_dir, &himax_black_screen_test_fops);
	if (himax_blackscreen_test == NULL) {
        E("create proc/touchpanel/blackscreen_test Failed!\n");
		goto fail_oppo_1;
	}
#ifdef HX_HIGH_SENSE
	himax_proc_HSEN_file = proc_create(HIMAX_PROC_HSEN_FILE, 0666,
									   himax_touch_proc_dir, &himax_proc_HSEN_ops);

	if (himax_proc_HSEN_file == NULL) {
		E(" %s: proc HSEN file create failed!\n", __func__);
		goto fail_1;
	}

#endif
#ifdef HX_SMART_WAKEUP
#ifdef HX_GESTURE_TRACK
	himax_proc_coor_file = proc_create(HIMAX_PROC_COOR_FILE, 0666,
									  himax_touch_proc_dir, &himax_proc_coor_ops);
	if (himax_proc_coor_file == NULL) {
		E(" %s: proc dbg_info file create failed!\n", __func__);
		goto fail_oppo_2;
	}
#endif
	himax_proc_double_tap_en_file = proc_create(HIMAX_PROC_DOUBLE_TAP_EN_FILE, 0666,
								  himax_touch_proc_dir, &himax_proc_double_tap_en_ops);
	if (himax_proc_double_tap_en_file == NULL) {
		E(" %s: proc double_tap_en file create failed!\n", __func__);
		goto fail_oppo_5;
	}

	himax_proc_SMWP_file = proc_create(HIMAX_PROC_SMWP_FILE, 0666,
									   himax_touch_proc_dir, &himax_proc_SMWP_ops);

	if (himax_proc_SMWP_file == NULL) {
		E(" %s: proc SMWP file create failed!\n", __func__);
		goto fail_2;
	}

	himax_proc_GESTURE_file = proc_create(HIMAX_PROC_GESTURE_FILE, 0666,
										  himax_touch_proc_dir, &himax_proc_Gesture_ops);

	if (himax_proc_GESTURE_file == NULL) {
		E(" %s: proc GESTURE file create failed!\n", __func__);
		goto fail_3;
	}
#endif
#ifdef HX_EDGE_LIMIT
	himax_proc_limit_enable_file = proc_create(HIMAX_PROC_LIMIT_ENABLE_FILE, (S_IWUSR | S_IRUGO | S_IWUGO),
										  himax_touch_proc_dir, &hx_proc_limit_control_ops);

	if (himax_proc_limit_enable_file == NULL) {
		E(" %s: proc limit enable file create failed!\n", __func__);
		goto fail_oppo_6;
	}
	himax_proc_limit_area_file = proc_create(HIMAX_PROC_LIMIT_AREA_FILE, (S_IWUSR | S_IRUGO | S_IWUGO),
										  himax_touch_proc_dir, &proc_limit_area_ops);

	if (himax_proc_limit_area_file == NULL) {
		E(" %s: proc limit area file create failed!\n", __func__);
		goto fail_oppo_7;
	}
#endif
	himax_proc_game_switch_en_file = proc_create(HIMAX_PROC_GAME_SWITCH_FILE, (S_IWUSR | S_IRUGO | S_IWUGO),
										  himax_touch_proc_dir, &hx_proc_game_switch_ops);

	if (himax_proc_game_switch_en_file == NULL) {
		E(" %s: proc game switch en file create failed!\n", __func__);
		goto fail_oppo_8;
	}

//#endif
	return 0;
	//wdd 18/07/25 unreachable code
	//remove_proc_entry(HIMAX_PROC_GAME_SWITCH_FILE, himax_touch_proc_dir);
fail_oppo_8:
#ifdef HX_EDGE_LIMIT
	remove_proc_entry(HIMAX_PROC_LIMIT_AREA_FILE, himax_touch_proc_dir);
fail_oppo_7:
	remove_proc_entry(HIMAX_PROC_LIMIT_ENABLE_FILE, himax_touch_proc_dir);
fail_oppo_6:
#endif

#ifdef HX_SMART_WAKEUP
	remove_proc_entry(HIMAX_PROC_GESTURE_FILE, himax_touch_proc_dir);
fail_3:
	remove_proc_entry(HIMAX_PROC_SMWP_FILE, himax_touch_proc_dir);
fail_2:
	remove_proc_entry(HIMAX_PROC_DOUBLE_TAP_EN_FILE, himax_touch_proc_dir);
fail_oppo_5:
#ifdef HX_GESTURE_TRACK
	remove_proc_entry(HIMAX_PROC_COOR_FILE, himax_touch_proc_dir);
fail_oppo_2:
#endif
#endif
#ifdef HX_HIGH_SENSE
	remove_proc_entry(HIMAX_PROC_HSEN_FILE, himax_touch_proc_dir);
fail_1:
#endif
	remove_proc_entry(HIMAX_PROC_BASELINE_TEST_FILE, himax_touch_proc_dir);
fail_oppo_1:
	return -ENOMEM;
}

void himax_common_proc_deinit(void)
{
	remove_proc_entry(HIMAX_PROC_GAME_SWITCH_FILE, himax_touch_proc_dir);
#ifdef HX_EDGE_LIMIT
	remove_proc_entry(HIMAX_PROC_LIMIT_AREA_FILE, himax_touch_proc_dir);
	remove_proc_entry(HIMAX_PROC_LIMIT_ENABLE_FILE, himax_touch_proc_dir);
#endif
	remove_proc_entry(HIMAX_PROC_BASELINE_TEST_FILE, himax_touch_proc_dir);
#ifdef HX_SMART_WAKEUP
#ifdef HX_GESTURE_TRACK
	remove_proc_entry(HIMAX_PROC_COOR_FILE, himax_proc_coor_file);
#endif
	remove_proc_entry(HIMAX_PROC_DOUBLE_TAP_EN_FILE, himax_touch_proc_dir);
	remove_proc_entry(HIMAX_PROC_GESTURE_FILE, himax_touch_proc_dir);
	remove_proc_entry(HIMAX_PROC_SMWP_FILE, himax_touch_proc_dir);
#endif
#ifdef HX_HIGH_SENSE
	remove_proc_entry(HIMAX_PROC_HSEN_FILE, himax_touch_proc_dir);
#endif
}

/* File node for SMWP and HSEN - End*/

int himax_input_register(struct himax_ts_data *ts)
{
	int ret = 0;
	ret = himax_dev_set(ts);

	if (ret < 0) {
		goto input_device_fail;
	}

	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
#if defined(HX_PLATFOME_DEFINE_KEY)
	himax_platform_key();
#else
	set_bit(KEY_BACK, ts->input_dev->keybit);
	set_bit(KEY_HOME, ts->input_dev->keybit);
	set_bit(KEY_MENU, ts->input_dev->keybit);
	set_bit(KEY_SEARCH, ts->input_dev->keybit);
#endif
#if defined(HX_SMART_WAKEUP) || defined(HX_PALM_REPORT)
	set_bit(KEY_POWER, ts->input_dev->keybit);
#endif
#if defined(HX_SMART_WAKEUP)
	set_bit(KEY_CUST_01, ts->input_dev->keybit);
	set_bit(KEY_CUST_02, ts->input_dev->keybit);
	set_bit(KEY_CUST_03, ts->input_dev->keybit);
	set_bit(KEY_CUST_04, ts->input_dev->keybit);
	set_bit(KEY_CUST_05, ts->input_dev->keybit);
	set_bit(KEY_CUST_06, ts->input_dev->keybit);
	set_bit(KEY_CUST_07, ts->input_dev->keybit);
	set_bit(KEY_CUST_08, ts->input_dev->keybit);
	set_bit(KEY_CUST_09, ts->input_dev->keybit);
	set_bit(KEY_CUST_10, ts->input_dev->keybit);
	set_bit(KEY_CUST_11, ts->input_dev->keybit);
	set_bit(KEY_CUST_12, ts->input_dev->keybit);
	set_bit(KEY_CUST_13, ts->input_dev->keybit);
	set_bit(KEY_CUST_14, ts->input_dev->keybit);
	set_bit(KEY_CUST_15, ts->input_dev->keybit);
#endif
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(KEY_APPSELECT, ts->input_dev->keybit);
	set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
#ifdef	HX_PROTOCOL_A
	/*ts->input_dev->mtsize = ts->nFinger_support;*/
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);
#else
	set_bit(MT_TOOL_FINGER, ts->input_dev->keybit);
#if defined(HX_PROTOCOL_B_3PA)
	input_mt_init_slots(ts->input_dev, ts->nFinger_support, INPUT_MT_DIRECT);
#else
	input_mt_init_slots(ts->input_dev, ts->nFinger_support);
#endif
#endif
	I("input_set_abs_params: mix_x %d, max_x %d, min_y %d, max_y %d\n",
	  ts->pdata->abs_x_min, ts->pdata->abs_x_max, ts->pdata->abs_y_min, ts->pdata->abs_y_max);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, ts->pdata->abs_x_min, ts->pdata->abs_x_max, ts->pdata->abs_x_fuzz, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, ts->pdata->abs_y_min, ts->pdata->abs_y_max, ts->pdata->abs_y_fuzz, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, ts->pdata->abs_pressure_min, ts->pdata->abs_pressure_max, ts->pdata->abs_pressure_fuzz, 0);
#ifndef	HX_PROTOCOL_A
	input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, ts->pdata->abs_pressure_min, ts->pdata->abs_pressure_max, ts->pdata->abs_pressure_fuzz, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, ts->pdata->abs_width_min, ts->pdata->abs_width_max, ts->pdata->abs_pressure_fuzz, 0);
#endif
/*	input_set_abs_params(ts->input_dev, ABS_MT_AMPLITUDE, 0, ((ts->pdata->abs_pressure_max << 16) | ts->pdata->abs_width_max), 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION, 0, (BIT(31) | (ts->pdata->abs_x_max << 16) | ts->pdata->abs_y_max), 0, 0);*/
	input_set_capability(ts->input_dev, EV_KEY, KEY_F4);

	if (himax_input_register_device(ts->input_dev) == 0) {
		return NO_ERR;
	} else {
		ret = INPUT_REGISTER_FAIL;
	}

input_device_fail:
	I("%s, input device register fail!\n", __func__);
	return ret;
}

static void calcDataSize(uint8_t finger_num)
{
	struct himax_ts_data *ts_data = private_ts;
	ts_data->coord_data_size = 4 * finger_num;
	ts_data->area_data_size = ((finger_num / 4) + (finger_num % 4 ? 1 : 0)) * 4;
	ts_data->coordInfoSize = ts_data->coord_data_size + ts_data->area_data_size + 4;
	ts_data->raw_data_frame_size = 128 - ts_data->coord_data_size - ts_data->area_data_size - 4 - 4 - 1;

	if (ts_data->raw_data_frame_size == 0) {
		E("%s: could NOT calculate! \n", __func__);
		return;
	}

	ts_data->raw_data_nframes  = ((uint32_t)ts_data->x_channel * ts_data->y_channel +
									ts_data->x_channel + ts_data->y_channel) / ts_data->raw_data_frame_size +
									(((uint32_t)ts_data->x_channel * ts_data->y_channel +
									ts_data->x_channel + ts_data->y_channel) % ts_data->raw_data_frame_size) ? 1 : 0;
	I("%s: coord_data_size: %d, area_data_size:%d, raw_data_frame_size:%d, raw_data_nframes:%d\n", __func__, ts_data->coord_data_size, ts_data->area_data_size, ts_data->raw_data_frame_size, ts_data->raw_data_nframes);
}

static void calculate_point_number(void)
{
	HX_TOUCH_INFO_POINT_CNT = ic_data->HX_MAX_PT * 4;

	if ((ic_data->HX_MAX_PT % 4) == 0) {
		HX_TOUCH_INFO_POINT_CNT += (ic_data->HX_MAX_PT / 4) * 4;
	} else {
		HX_TOUCH_INFO_POINT_CNT += ((ic_data->HX_MAX_PT / 4) + 1) * 4;
	}
}
#if defined(HX_AUTO_UPDATE_FW)
static int himax_auto_update_check(void)
{
	I("%s:Entering!\n", __func__);
	if (g_core_fp.fp_fw_ver_bin() == 0) {
		if (((ic_data->vendor_fw_ver < g_i_FW_VER) || (ic_data->vendor_config_ver < g_i_CFG_VER))) {
			I("Need to update!\n");
			return NO_ERR;
		}	else {
			E("No need to update!\n");
			return 1;
		}
	} else {
		E("FW bin fail!\n");
		return 1;
	}
}
static int i_get_FW(void)
{
	int ret = 0;
	const struct firmware *image = NULL;

	if ( hx_ctpmodule == 0 ) {
		I("file name = %s\n", i_CTPM_firmware_name);
		ret = request_firmware (&fw_entry, i_CTPM_firmware_name, private_ts->dev);
	} else {
		I("file name = %s\n", i_CTPM_firmware_name_TCL);
		ret = request_firmware (&fw_entry, i_CTPM_firmware_name_TCL, private_ts->dev);

	}
	if (ret < 0) {
		E("%s,fail in line%d error code=%d\n", __func__, __LINE__, ret);
		return OPEN_FILE_FAIL;
	}

	if (image != NULL) {
		i_CTPM_FW_len = image->size;
		i_CTPM_FW = kzalloc(sizeof(char)*i_CTPM_FW_len, GFP_KERNEL);
		memcpy(i_CTPM_FW, image->data, sizeof(char)*i_CTPM_FW_len);
	} else {
		I("%s: i_CTPM_FW = NULL\n", __func__);
		return OPEN_FILE_FAIL;
	}

	release_firmware(image);
	ret = NO_ERR;
	return ret;
}
static int i_update_FW(void)
{
	int upgrade_times = 0;

	uint8_t ret = 0, result = 0;

	himax_int_enable(0);


update_retry:

	if (i_CTPM_FW_len == FW_SIZE_32k) {
		ret = g_core_fp.fp_fts_ctpm_fw_upgrade_with_sys_fs_32k(i_CTPM_FW, i_CTPM_FW_len, false);
	} else if (i_CTPM_FW_len == FW_SIZE_60k) {
		ret = g_core_fp.fp_fts_ctpm_fw_upgrade_with_sys_fs_60k(i_CTPM_FW, i_CTPM_FW_len, false);
	} else if (i_CTPM_FW_len == FW_SIZE_64k) {
		ret = g_core_fp.fp_fts_ctpm_fw_upgrade_with_sys_fs_64k(i_CTPM_FW, i_CTPM_FW_len, false);
	} else if (i_CTPM_FW_len == FW_SIZE_124k) {
		ret = g_core_fp.fp_fts_ctpm_fw_upgrade_with_sys_fs_124k(i_CTPM_FW, i_CTPM_FW_len, false);
	} else if (i_CTPM_FW_len == FW_SIZE_128k) {
		ret = g_core_fp.fp_fts_ctpm_fw_upgrade_with_sys_fs_128k(i_CTPM_FW, i_CTPM_FW_len, false);
	}

	if (ret == 0) {
		upgrade_times++;
		E("%s: TP upgrade error, upgrade_times = %d\n", __func__, upgrade_times);

		if (upgrade_times < 3) {
			goto update_retry;
		}	else {
			result = -1;
		} /*upgrade fail*/
	} else {
		g_core_fp.fp_read_FW_ver();
		g_core_fp.fp_touch_information();
		result = 1;/*upgrade success*/
		I("%s: TP upgrade OK\n", __func__);
	}

	kfree(i_CTPM_FW);

#ifdef HX_RST_PIN_FUNC
	g_core_fp.fp_ic_reset(true, false);
#else
	g_core_fp.fp_sense_on(0x00);
#endif
	himax_int_enable(1);
	return result;
}
#endif


/*static int himax_loadSensorConfig(struct himax_i2c_platform_data *pdata)
{
	I("%s: initialization complete\n", __func__);
	return NO_ERR;
} */

#ifdef HX_ESD_RECOVERY
static void himax_esd_hw_reset(void)
{
	int result = 0;
	int retry = 10;
	if (g_ts_dbg != 0)
		I("%s: Entering \n", __func__);

	I("START_Himax TP: ESD - Reset\n");

	if (private_ts->in_baseline_test == 1) {
		I("In self test , not  TP: ESD - Reset\n");
		return;
	}


#ifdef ODM_WT_EDIT
//Tianchen.Zhao@ODM_RH.TP Porting
#ifdef HX_ZERO_FLASH
	I("It will update fw after esd event in zero flash mode!\n");
	do {
		g_core_fp.fp_esd_ic_reset();
		if (g_core_fp.fp_0f_operation_dirly())
			continue;
		if (!g_core_fp.fp_reload_disable(0))
			break;
	} while (result != 0 && --retry > 0);
	if (retry <= 0) {
		E("Retry 10 times failed, do not sense on!\n");
		goto ESCAPE_0F_UPDATE;
	}
#endif
#endif
	g_core_fp.fp_sense_on(0x00);
	himax_report_all_leave_event(private_ts);
	himax_int_enable(1);

ESCAPE_0F_UPDATE:
	I("END_Himax TP: ESD - Reset\n");
}
#endif

#ifdef HX_SMART_WAKEUP
#ifdef HX_GESTURE_TRACK
static void gest_pt_log_coordinate(int rx, int tx)
{
	/*driver report x y with range 0 - 255 , we scale it up to x/y pixel*/
	gest_pt_x[gest_pt_cnt] = rx * (ic_data->HX_X_RES) / 255;
	gest_pt_y[gest_pt_cnt] = tx * (ic_data->HX_Y_RES) / 255;
}
#endif
static int himax_wake_event_parse(struct himax_ts_data *ts, int ts_status)
{
	uint8_t *buf;
#ifdef HX_GESTURE_TRACK
#if 0
	int tmp_max_x = 0x00, tmp_min_x = 0xFFFF, tmp_max_y = 0x00, tmp_min_y = 0xFFFF;
#endif
	int gest_len;
#endif
	int i = 0, check_FC = 0, gesture_flag = 0;

	if (g_ts_dbg != 0)
		I("%s: Entering!, ts_status=%d\n", __func__, ts_status);

	buf = kzalloc(hx_touch_data->event_size * sizeof(uint8_t), GFP_KERNEL);
	if (buf == NULL) {
		return -ENOMEM;
	}

	memcpy(buf, hx_touch_data->hx_event_buf, hx_touch_data->event_size);

	for (i = 0; i < GEST_PTLG_ID_LEN; i++) {
		if (check_FC == 0) {
			if ((buf[0] != 0x00) && ((buf[0] <= 0x0F) || (buf[0] == 0x80 || (buf[0] == 0x81))
			 || (buf[0] == 0x1D) || (buf[0] == 0x2D) || (buf[0] == 0x3D) || (buf[0] == 0x1F)
			 || (buf[0] == 0x2F) || (buf[0] >= 0x51 && buf[0] <= 0x54))) {
				check_FC = 1;
				gesture_flag = buf[i];
			} else {
				check_FC = 0;
				I("ID START at %x , value = %x skip the event\n", i, buf[i]);
				break;
			}
		} else {
			if (buf[i] != gesture_flag) {
				check_FC = 0;
				I("ID NOT the same %x != %x So STOP parse event\n", buf[i], gesture_flag);
				break;
			}
		}

		I("0x%2.2X ", buf[i]);

		if (i % 8 == 7) {
			I("\n");
		}
	}

	I("Himax gesture_flag= %x\n", gesture_flag);
	I("Himax check_FC is %d\n", check_FC);

	if (check_FC == 0) {
		kfree(buf);
		return 0;
	}

	if (buf[GEST_PTLG_ID_LEN] != GEST_PTLG_HDR_ID1 ||
		buf[GEST_PTLG_ID_LEN + 1] != GEST_PTLG_HDR_ID2) {
		kfree(buf);
		return 0;
	}

#ifdef HX_GESTURE_TRACK

	if (buf[GEST_PTLG_ID_LEN] == GEST_PTLG_HDR_ID1 &&
		buf[GEST_PTLG_ID_LEN + 1] == GEST_PTLG_HDR_ID2) {
		gest_len = buf[GEST_PTLG_ID_LEN + 2];
		I("gest_len = %d \n", gest_len);
		i = 0;
		gest_pt_cnt = 0;
		I("gest doornidate start \n %s", __func__);
#if 1 /*oppo gesture parsing*/

		for (i = 0; i < 14; i++) {/*default value initialize*/
			hx_gesture_coor[i] = 0;
			if (i == 13)
				hx_gesture_coor[i] = 1;
		}
		hx_gesture_coor[0] = buf[GEST_PTLG_ID_LEN + 4];
		for (i = 0; i < 14; i++) {
			I("buf[%d]=%d \n", GEST_PTLG_ID_LEN + 4 + i, buf[GEST_PTLG_ID_LEN + 4 + i + 1]);
		}

		i = 0;
		
		/*wdd 18/07/25 over run*/
		gest_len = gest_len > 7 ? 7 : gest_len;
		while (i < gest_len) {
			if (i != 6) {
				gest_pt_log_coordinate(buf[GEST_PTLG_ID_LEN + 5 + i * 2], buf[GEST_PTLG_ID_LEN + 5 + i * 2 + 1]);
				hx_gesture_coor[i * 2 + 1] = gest_pt_x[gest_pt_cnt];
				hx_gesture_coor[i * 2 + 2] = gest_pt_y[gest_pt_cnt];
				I("hx_gest_coor[%d]=%d \n", i * 2 + 1, hx_gesture_coor[i * 2 + 1]);
				I("hx_gest_coor[%d]=%d \n", i * 2 + 2, hx_gesture_coor[i * 2 + 2]);
				gest_pt_cnt++;
			} else {
				hx_gesture_coor[i * 2 + 1] = buf[GEST_PTLG_ID_LEN + 5 + i * 2];
				I("hx_gest_coor[%d]=%d \n", i * 2 + 1, hx_gesture_coor[i * 2 + 1]);
			}
			i++;
		}
#else
		while (i < (gest_len + 1) / 2) {
			gest_pt_log_coordinate(buf[GEST_PTLG_ID_LEN + 4 + i * 2], buf[GEST_PTLG_ID_LEN + 4 + i * 2 + 1]);
			i++;
			I("gest_pt_x[%d]=%d \n", gest_pt_cnt, gest_pt_x[gest_pt_cnt]);
			I("gest_pt_y[%d]=%d \n", gest_pt_cnt, gest_pt_y[gest_pt_cnt]);
			gest_pt_cnt += 1;
		}

		if (gest_pt_cnt) {
			for (i = 0; i < gest_pt_cnt; i++) {
				if (tmp_max_x < gest_pt_x[i])
					tmp_max_x = gest_pt_x[i];
				if (tmp_min_x > gest_pt_x[i])
					tmp_min_x = gest_pt_x[i];
				if (tmp_max_y < gest_pt_y[i])
					tmp_max_y = gest_pt_y[i];
				if (tmp_min_y > gest_pt_y[i])
					tmp_min_y = gest_pt_y[i];
			}

			I("gest_point x_min= %d, x_max= %d, y_min= %d, y_max= %d\n", tmp_min_x, tmp_max_x, tmp_min_y, tmp_max_y);
			gest_start_x = gest_pt_x[0];
			hx_gesture_coor[0] = gest_start_x;
			gest_start_y = gest_pt_y[0];
			hx_gesture_coor[1] = gest_start_y;
			gest_end_x = gest_pt_x[gest_pt_cnt - 1];
			hx_gesture_coor[2] = gest_end_x;
			gest_end_y = gest_pt_y[gest_pt_cnt - 1];
			hx_gesture_coor[3] = gest_end_y;
			gest_width = tmp_max_x - tmp_min_x;
			hx_gesture_coor[4] = gest_width;
			gest_height = tmp_max_y - tmp_min_y;
			hx_gesture_coor[5] = gest_height;
			gest_mid_x = (tmp_max_x + tmp_min_x) / 2;
			hx_gesture_coor[6] = gest_mid_x;
			gest_mid_y = (tmp_max_y + tmp_min_y) / 2;
			hx_gesture_coor[7] = gest_mid_y;
			hx_gesture_coor[8] = gest_mid_x;/*gest_up_x*/
			hx_gesture_coor[9] = gest_mid_y - gest_height / 2; /*gest_up_y*/
			hx_gesture_coor[10] = gest_mid_x;/*gest_down_x*/
			hx_gesture_coor[11] = gest_mid_y + gest_height / 2;	/*gest_down_y*/
			hx_gesture_coor[12] = gest_mid_x - gest_width / 2;	/*gest_left_x*/
			hx_gesture_coor[13] = gest_mid_y;	/*gest_left_y*/
			hx_gesture_coor[14] = gest_mid_x + gest_width / 2;	/*gest_right_x*/
			hx_gesture_coor[15] = gest_mid_y; /*gest_right_y*/
		}
#endif
	}

#endif

	kfree(buf);
#if 0
	if (gesture_flag != 0x80) {
		if (!ts->gesture_cust_en[gesture_flag]) {
			I("%s NOT report customer key \n ", __func__);
			g_target_report_data->SMWP_event_chk = 0;
			return 0;/* NOT report customer key */
		}
	} else {
		if (!ts->gesture_cust_en[0]) {
			I("%s NOT report report double click \n", __func__);
			g_target_report_data->SMWP_event_chk = 0;
			return 0;/* NOT report power key */
		}
	}

	if (gesture_flag == 0x80) {
		g_target_report_data->SMWP_event_chk = EV_GESTURE_PWR;
		return EV_GESTURE_PWR;
	} else {
		g_target_report_data->SMWP_event_chk = gesture_flag;
		return gesture_flag;
	}
#endif
	g_target_report_data->SMWP_event_chk = gesture_flag;
	return gesture_flag;


}

static void himax_wake_event_report(void)
{
	int ret_event = g_target_report_data->SMWP_event_chk;
	int KEY_EVENT = 0;

	if (g_ts_dbg != 0)
		I("%s: Entering! \n", __func__);

#if 1
	KEY_EVENT = KEY_F4;
#else
	switch (ret_event) {
	case EV_GESTURE_PWR:
		KEY_EVENT = KEY_POWER;
		break;

	case EV_GESTURE_01:
		KEY_EVENT = KEY_CUST_01;
		break;

	case EV_GESTURE_02:
		KEY_EVENT = KEY_CUST_02;
		break;

	case EV_GESTURE_03:
		KEY_EVENT = KEY_CUST_03;
		break;

	case EV_GESTURE_04:
		KEY_EVENT = KEY_CUST_04;
		break;

	case EV_GESTURE_05:
		KEY_EVENT = KEY_CUST_05;
		break;

	case EV_GESTURE_06:
		KEY_EVENT = KEY_CUST_06;
		break;

	case EV_GESTURE_07:
		KEY_EVENT = KEY_CUST_07;
		break;

	case EV_GESTURE_08:
		KEY_EVENT = KEY_CUST_08;
		break;

	case EV_GESTURE_09:
		KEY_EVENT = KEY_CUST_09;
		break;

	case EV_GESTURE_10:
		KEY_EVENT = KEY_CUST_10;
		break;

	case EV_GESTURE_11:
		KEY_EVENT = KEY_CUST_11;
		break;

	case EV_GESTURE_12:
		KEY_EVENT = KEY_CUST_12;
		break;

	case EV_GESTURE_13:
		KEY_EVENT = KEY_CUST_13;
		break;

	case EV_GESTURE_14:
		KEY_EVENT = KEY_CUST_14;
		break;

	case EV_GESTURE_15:
		KEY_EVENT = KEY_CUST_15;
		break;
	}
#endif

	if (ret_event) {
		I(" %s SMART WAKEUP KEY event %x press\n", __func__, KEY_EVENT);
		input_report_key(private_ts->input_dev, KEY_EVENT, 1);
		input_sync(private_ts->input_dev);
		I(" %s SMART WAKEUP KEY event %x release\n", __func__, KEY_EVENT);
		input_report_key(private_ts->input_dev, KEY_EVENT, 0);
		input_sync(private_ts->input_dev);
		FAKE_POWER_KEY_SEND = true;
#if 0 /*def HX_GESTURE_TRACK*/
		I("gest_start_x= %d, gest_start_y= %d, gest_end_x= %d, gest_end_y= %d\n", gest_start_x, gest_start_y,
		  gest_end_x, gest_end_y);
		I("gest_width= %d, gest_height= %d, gest_mid_x= %d, gest_mid_y= %d\n", gest_width, gest_height,
		  gest_mid_x, gest_mid_y);
		I("gest_up_x= %d, gest_up_y= %d, gest_down_x= %d, gest_down_y= %d\n", hx_gesture_coor[8], hx_gesture_coor[9],
		  hx_gesture_coor[10], hx_gesture_coor[11]);
		I("gest_left_x= %d, gest_left_y= %d, gest_right_x= %d, gest_right_y= %d\n", hx_gesture_coor[12], hx_gesture_coor[13],
		  hx_gesture_coor[14], hx_gesture_coor[15]);
#endif
		g_target_report_data->SMWP_event_chk = 0;
	}
}

#endif

int himax_report_data_init(void)
{
	if (hx_touch_data->hx_coord_buf != NULL) {
		kfree(hx_touch_data->hx_coord_buf);
	}

	if (hx_touch_data->hx_rawdata_buf != NULL) {
		kfree(hx_touch_data->hx_rawdata_buf);
	}

#if defined(HX_SMART_WAKEUP)
	hx_touch_data->event_size = g_core_fp.fp_get_touch_data_size();

	if (hx_touch_data->hx_event_buf != NULL) {
		kfree(hx_touch_data->hx_event_buf);
	}

#endif
	hx_touch_data->touch_all_size = g_core_fp.fp_get_touch_data_size();
	hx_touch_data->raw_cnt_max = ic_data->HX_MAX_PT / 4;
	hx_touch_data->raw_cnt_rmd = ic_data->HX_MAX_PT % 4;
	/* more than 4 fingers */
	if (hx_touch_data->raw_cnt_rmd != 0x00) {
		hx_touch_data->rawdata_size = g_core_fp.fp_cal_data_len(hx_touch_data->raw_cnt_rmd, ic_data->HX_MAX_PT, hx_touch_data->raw_cnt_max);
		hx_touch_data->touch_info_size = (ic_data->HX_MAX_PT + hx_touch_data->raw_cnt_max + 2) * 4;
	} else { /* less than 4 fingers */
		hx_touch_data->rawdata_size = g_core_fp.fp_cal_data_len(hx_touch_data->raw_cnt_rmd, ic_data->HX_MAX_PT, hx_touch_data->raw_cnt_max);
		hx_touch_data->touch_info_size = (ic_data->HX_MAX_PT + hx_touch_data->raw_cnt_max + 1) * 4;
	}

	if ((ic_data->HX_TX_NUM * ic_data->HX_RX_NUM + ic_data->HX_TX_NUM + ic_data->HX_RX_NUM) % hx_touch_data->rawdata_size == 0) {
		hx_touch_data->rawdata_frame_size = (ic_data->HX_TX_NUM * ic_data->HX_RX_NUM + ic_data->HX_TX_NUM + ic_data->HX_RX_NUM) / hx_touch_data->rawdata_size;
	} else {
		hx_touch_data->rawdata_frame_size = (ic_data->HX_TX_NUM * ic_data->HX_RX_NUM + ic_data->HX_TX_NUM + ic_data->HX_RX_NUM) / hx_touch_data->rawdata_size + 1;
	}

	I("%s: rawdata_frame_size = %d\n", __func__, hx_touch_data->rawdata_frame_size);
	I("%s: ic_data->HX_MAX_PT:%d, hx_raw_cnt_max:%d, hx_raw_cnt_rmd:%d, g_hx_rawdata_size:%d, hx_touch_data->touch_info_size:%d\n", __func__, ic_data->HX_MAX_PT, hx_touch_data->raw_cnt_max, hx_touch_data->raw_cnt_rmd, hx_touch_data->rawdata_size, hx_touch_data->touch_info_size);
	hx_touch_data->hx_coord_buf = kzalloc(sizeof(uint8_t) * (hx_touch_data->touch_info_size), GFP_KERNEL);

	if (hx_touch_data->hx_coord_buf == NULL) {
		goto mem_alloc_fail;
	}

	if (g_target_report_data == NULL) {
		g_target_report_data = kzalloc(sizeof(struct himax_target_report_data), GFP_KERNEL);
		if (g_target_report_data == NULL)
			goto mem_alloc_fail;
		g_target_report_data->x = kzalloc(sizeof(int)*(ic_data->HX_MAX_PT), GFP_KERNEL);
		if (g_target_report_data->x == NULL)
			goto mem_alloc_fail;
		g_target_report_data->y = kzalloc(sizeof(int)*(ic_data->HX_MAX_PT), GFP_KERNEL);
		if (g_target_report_data->y == NULL)
			goto mem_alloc_fail;
		g_target_report_data->w = kzalloc(sizeof(int)*(ic_data->HX_MAX_PT), GFP_KERNEL);
		if (g_target_report_data->w == NULL)
			goto mem_alloc_fail;
		g_target_report_data->finger_id = kzalloc(sizeof(int)*(ic_data->HX_MAX_PT), GFP_KERNEL);
		if (g_target_report_data->finger_id == NULL)
			goto mem_alloc_fail;
	}
#ifdef HX_SMART_WAKEUP
	g_target_report_data->SMWP_event_chk = 0;
#endif

	hx_touch_data->hx_rawdata_buf = kzalloc(sizeof(uint8_t) * (hx_touch_data->touch_all_size - hx_touch_data->touch_info_size), GFP_KERNEL);

	if (hx_touch_data->hx_rawdata_buf == NULL) {
		goto mem_alloc_fail;
	}

#if defined(HX_SMART_WAKEUP)
	hx_touch_data->hx_event_buf = kzalloc(sizeof(uint8_t) * (hx_touch_data->event_size), GFP_KERNEL);

	if (hx_touch_data->hx_event_buf == NULL) {
		goto mem_alloc_fail;
	}

#endif
	return NO_ERR;
mem_alloc_fail:
	/*wdd 18/07/25 NULL ptr*/
	if(g_target_report_data != NULL)
	{
		if(g_target_report_data->x != NULL)
			kfree(g_target_report_data->x);
		
		if(g_target_report_data->y != NULL)
			kfree(g_target_report_data->y);
		
		if(g_target_report_data->w != NULL)
			kfree(g_target_report_data->w);
		
		if(g_target_report_data->finger_id != NULL)
			kfree(g_target_report_data->finger_id);
		
		kfree(g_target_report_data);
		g_target_report_data = NULL;
	}
	if(hx_touch_data->hx_coord_buf != NULL)
		kfree(hx_touch_data->hx_coord_buf);
	if(hx_touch_data->hx_rawdata_buf != NULL)
		kfree(hx_touch_data->hx_rawdata_buf);
#if defined(HX_SMART_WAKEUP)
	if(hx_touch_data->hx_event_buf != NULL)
		kfree(hx_touch_data->hx_event_buf);
#endif
	I("%s: Memory allocate fail!\n", __func__);
	return MEM_ALLOC_FAIL;
}

/*start ts_work*/
#ifdef HX_EDGE_LIMIT
static bool corner_point_process(struct himax_ts_data *ts, struct corner_info *corner, struct point_info *points, int i)
{
		int j;
		int flag = 0;

		if (ts->limit_corner) {
				/* /proc/touchpanel/oppo_tp_limit_enable control bit1*/
				if ((ts->limit_corner & (1 << CORNER_TOPLEFT)) && (points[i].x < ts->edge_limit.left_x1 && points[i].y < ts->edge_limit.left_y1)) {
						points[i].type  = AREA_CORNER;
						//I("%s:0 bit1: points[%d].type = %d!\n", __func__, i, points[i].type);

						/*if (ts->edge_limit.in_which_area == AREA_NORMAL)
								return true;*/

						corner[CORNER_TOPLEFT].id = i;
						corner[CORNER_TOPLEFT].point = points[i];
						corner[CORNER_TOPLEFT].flag = true;
						flag = 1;

						/*ts->edge_limit.in_which_area = points[i].type;*/
				}
				 /* /proc/touchpanel/oppo_tp_limit_enable control bit2*/
				/*if ((ts->limit_corner & (1 << CORNER_TOPRIGHT))  && (points[i].x < ts->edge_limit.left_x1 && points[i].y > ts->edge_limit.right_y1)) {*/
					if ((ts->limit_corner & (1 << CORNER_TOPRIGHT))  && (points[i].x > ts->edge_limit.right_x1 && points[i].y < ts->edge_limit.right_y1)) {
						points[i].type  = AREA_CORNER;
						//I("%s:0 bit2: points[%d].type = %d!\n", __func__, i, points[i].type);
						/*if (ts->edge_limit.in_which_area == AREA_NORMAL)
								return true;*/

						corner[CORNER_TOPRIGHT].id = i;
						corner[CORNER_TOPRIGHT].point = points[i];
						corner[CORNER_TOPRIGHT].flag = true;
						flag = 1;

						/*ts->edge_limit.in_which_area = points[i].type;*/
				}
				 /* /proc/touchpanel/oppo_tp_limit_enable control bit3*/
				/*if ((ts->limit_corner & (1 << CORNER_BOTTOMLEFT))  && (points[i].x > ts->edge_limit.right_x1 && points[i].y < ts->edge_limit.left_y1)) {*/
					if ((ts->limit_corner & (1 << CORNER_BOTTOMLEFT))  && (points[i].x < ts->edge_limit.left_x2 && points[i].y > ts->edge_limit.left_y2)) {
						points[i].type  = AREA_CORNER;
						//I("%s:0 bit3: points[%d].type = %d!\n", __func__, i, points[i].type);
						/*if (ts->edge_limit.in_which_area == AREA_NORMAL)
								return true;*/

						corner[CORNER_BOTTOMLEFT].id = i;
						corner[CORNER_BOTTOMLEFT].point = points[i];
						corner[CORNER_BOTTOMLEFT].flag = true;
						flag = 1;

						/*ts->edge_limit.in_which_area = points[i].type;*/
				}
				 /* /proc/touchpanel/oppo_tp_limit_enable control bit4*/
				/*if ((ts->limit_corner & (1 << CORNER_BOTTOMRIGHT))  && (points[i].x > ts->edge_limit.right_x1 && points[i].y > ts->edge_limit.right_y1)) {*/
					if ((ts->limit_corner & (1 << CORNER_BOTTOMRIGHT))  && (points[i].x > ts->edge_limit.right_x2 && points[i].y > ts->edge_limit.right_y2)) {
						points[i].type  = AREA_CORNER;
						//I("%s:0 bit4: points[%d].type = %d!\n", __func__, i, points[i].type);
						/*if (ts->edge_limit.in_which_area == AREA_NORMAL)
							return true;*/

						corner[CORNER_BOTTOMRIGHT].id = i;
						corner[CORNER_BOTTOMRIGHT].point = points[i];
						corner[CORNER_BOTTOMRIGHT].flag = true;
						flag = 1;

						/*ts->edge_limit.in_which_area = points[i].type;*/
				}
				/*If coordinate is not corner point, then release all corner points*/
				if (points[i].type != AREA_CORNER) {
						if (ts->edge_limit.in_which_area == AREA_CORNER) {
								for (j = 0; j < 4; j++) {
										if (corner[j].flag && (ts->protocol_type == PROTOCOL_TYPE_B)) {
												input_mt_slot(ts->input_dev, corner[j].id);
												input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);

										}
								}
						}
				}
				if (flag == 0) {
					ts->edge_limit.in_which_area = AREA_NORMAL;
				} else {
					//I("%s: x = %d, y = %d\n", __func__, points[i].x, points[i].y);
					ts->edge_limit.in_which_area = AREA_CORNER;
				}
		}
		return false;
}
#endif


#if defined(HX_USB_DETECT_GLOBAL)
extern int usb_state_ctp;
void himax_cable_detect_func(bool force_renew)
{
	struct himax_ts_data *ts;
	u32 connect_status = 0;
	connect_status = usb_state_ctp;/* upmu_is_chr_det(); */
	ts = private_ts;

	/* I("Touch: cable status=%d, cable_config=%p, usb_connected=%d \n", connect_status, ts->cable_config, ts->usb_connected); */
	if (ts->cable_config) {
		if (((!!connect_status) != ts->usb_connected) || force_renew) {
			if (!!connect_status) {
				ts->cable_config[1] = 0x01;
				ts->usb_connected = 0x01;
			} else {
				ts->cable_config[1] = 0x00;
				ts->usb_connected = 0x00;
			}

			g_core_fp.fp_usb_detect_set(ts->cable_config);
			I("%s: Cable status change: 0x%2.2X\n", __func__, ts->usb_connected);
		}

		/* else */
		/* 	I("%s: Cable status is the same as previous one, ignore.\n", __func__); */
	}
}
#endif

static int himax_ts_work_status(struct himax_ts_data *ts)
{
	/* 1: normal, 2:SMWP */
	int result = HX_REPORT_COORD;

	hx_touch_data->diag_cmd = ts->diag_cmd;
	if (hx_touch_data->diag_cmd)
		result = HX_REPORT_COORD_RAWDATA;

#ifdef HX_SMART_WAKEUP
	if (atomic_read(&ts->suspend_mode) && /*(!FAKE_POWER_KEY_SEND) &&*/ (ts->SMWP_enable) && (!hx_touch_data->diag_cmd))
		result = HX_REPORT_SMWP_EVENT;
#endif
	/* I("Now Status is %d\n", result); */
	return result;
}

static int himax_touch_get(struct himax_ts_data *ts, uint8_t *buf, int ts_path, int ts_status)
{
	int ret= 0;
	int retry_spi = 20;

	if (g_ts_dbg != 0)
		I("%s: Entering, ts_status=%d! \n", __func__, ts_status);

	switch (ts_path) {
	/*normal*/
	case HX_REPORT_COORD:
		if ((HX_HW_RESET_ACTIVATE)
#ifdef HX_ESD_RECOVERY
			|| (HX_ESD_RESET_ACTIVATE)
#endif
			) {
			if (!g_core_fp.fp_read_event_stack(buf, 128)) {
				E("%s: can't read data from chip!\n", __func__);
				ts_status = HX_TS_GET_DATA_FAIL;
				goto END_FUNCTION;
			}
			break;
		} else {
			if (!g_core_fp.fp_read_event_stack(buf, hx_touch_data->touch_info_size)) {
				E("%s: can't read data from chip!\n", __func__);
				ts_status = HX_TS_GET_DATA_FAIL;
				goto END_FUNCTION;
			}
			break;
		}
#if defined(HX_SMART_WAKEUP)

	/*SMWP*/
	case HX_REPORT_SMWP_EVENT:
		do{
			ret = g_core_fp.fp_burst_enable(0);
			if (ret < 0) {
				retry_spi--;
				msleep(20);
				I("%s:retry_spi=%d! \n", __func__, retry_spi);	
			}	
		} while( ret < 0 && retry_spi > 0);

		if (!g_core_fp.fp_read_event_stack(buf, hx_touch_data->event_size)) {
			E("%s: can't read data from chip!\n", __func__);
			ts_status = HX_TS_GET_DATA_FAIL;
			goto END_FUNCTION;
		}
		break;
#endif
	case HX_REPORT_COORD_RAWDATA:
		if (!g_core_fp.fp_read_event_stack(buf, 128)) {
			E("%s: can't read data from chip!\n", __func__);
			ts_status = HX_TS_GET_DATA_FAIL;
			goto END_FUNCTION;
		}
		break;
	default:
		break;
	}

END_FUNCTION:
	return ts_status;
}

/* start error_control*/
static int himax_checksum_cal(struct himax_ts_data *ts, uint8_t *buf, int ts_path, int ts_status)
{
	uint16_t check_sum_cal = 0;
	int32_t	i = 0;
	int length = 0;
	int zero_cnt = 0;
	int ret_val = ts_status;

	if (g_ts_dbg != 0)
		I("%s: Entering, ts_status=%d! \n", __func__, ts_status);

	/* Normal */
	switch (ts_path) {
	case HX_REPORT_COORD:
		length = hx_touch_data->touch_info_size;
		break;
#if defined(HX_SMART_WAKEUP)
/* SMWP */
	case HX_REPORT_SMWP_EVENT:
		length = (GEST_PTLG_ID_LEN + GEST_PTLG_HDR_LEN);
		break;
#endif
	case HX_REPORT_COORD_RAWDATA:
		length = hx_touch_data->touch_info_size;
		break;
	default:
		I("%s, Neither Normal Nor SMWP error!\n", __func__);
		ret_val = HX_PATH_FAIL;
		goto END_FUNCTION;
	}

	for (i = 0; i < length; i++) {
		check_sum_cal += buf[i];
		if (buf[i] == 0x00)
			zero_cnt++;
	}

	if (check_sum_cal % 0x100 != 0) {
		I("[HIMAX TP MSG] checksum fail : check_sum_cal: 0x%02X\n", check_sum_cal);
		ret_val = HX_CHKSUM_FAIL;
	} else if (zero_cnt == length) {
		I("[HIMAX TP MSG] All Zero event\n");
		ret_val = HX_CHKSUM_FAIL;
	}

END_FUNCTION:
	if (g_ts_dbg != 0)
		I("%s: END, ret_val=%d! \n", __func__, ret_val);
	return ret_val;
}

#ifdef HX_ESD_RECOVERY
#ifdef HX_ZERO_FLASH
void hx_update_dirly_0f(void)
{
	I("It will update fw after esd event in zero flash mode!\n");
	g_core_fp.fp_0f_operation_dirly();
}
#endif
static int himax_ts_event_check(struct himax_ts_data *ts, uint8_t *buf, int ts_path, int ts_status)
{
	int hx_EB_event = 0;
	int hx_EC_event = 0;
	int hx_ED_event = 0;
	int hx_esd_event = 0;
	int hx_zero_event = 0;
	int shaking_ret = 0;

	int32_t	loop_i = 0;
	int length = 0;
	int ret_val = ts_status;

	if (g_ts_dbg != 0)
		I("%s: Entering, ts_status=%d! \n", __func__, ts_status);

	/* Normal */
	switch (ts_path) {
	case HX_REPORT_COORD:
		length = hx_touch_data->touch_info_size;
		break;
#if defined(HX_SMART_WAKEUP)
/* SMWP */
	case HX_REPORT_SMWP_EVENT:
		length = (GEST_PTLG_ID_LEN + GEST_PTLG_HDR_LEN);
		break;
#endif
	case HX_REPORT_COORD_RAWDATA:
		length = hx_touch_data->touch_info_size;
		break;
	default:
		I("%s, Neither Normal Nor SMWP error!\n", __func__);
		ret_val = HX_PATH_FAIL;
		goto END_FUNCTION;
	}

	if (g_ts_dbg != 0)
		I("Now Path=%d, Now status=%d, length=%d\n", ts_path, ts_status, length);

	for (loop_i = 0; loop_i < length; loop_i++) {
		if (ts_path == HX_REPORT_COORD || ts_path == HX_REPORT_COORD_RAWDATA) {
			/* case 1 ESD recovery flow */
			if (buf[loop_i] == 0xEB) {
				hx_EB_event++;
			} else if (buf[loop_i] == 0xEC) {
				hx_EC_event++;
			} else if (buf[loop_i] == 0xED) {
				hx_ED_event++;
			} else if (buf[loop_i] == 0x00) { /* case 2 ESD recovery flow-Disable */
				hx_zero_event++;
			} else {
				hx_EB_event = 0;
				hx_EC_event = 0;
				hx_ED_event = 0;
				hx_zero_event = 0;
			}
		}
	}

	if (hx_EB_event == length) {
		hx_esd_event = length;
		hx_EB_event_flag++;
		I("[HIMAX TP MSG]: ESD event checked - ALL 0xEB.\n");
	} else if (hx_EC_event == length) {
		hx_esd_event = length;
		hx_EC_event_flag++;
		I("[HIMAX TP MSG]: ESD event checked - ALL 0xEC.\n");
	} else if (hx_ED_event == length) {
		hx_esd_event = length;
		hx_ED_event_flag++;
		I("[HIMAX TP MSG]: ESD event checked - ALL 0xED.\n");
	}
#ifdef HX_ZERO_FLASH
		else if (hx_zero_event == length && hx_ctpmodule == 0) {
		/*check zero flash status*/
		if (g_core_fp.fp_0f_esd_check() < 0 ) {
			g_zero_event_count = 6;
			//g_zero_event_count++;
			I("[HIMAX TP MSG]: ESD event checked - ALL Zero in ZF.\n");
		} else {
			I("[HIMAX TP MSG]: Status check pass in ZF.\n");
		}
	}
#endif
	else {
		hx_esd_event = 0;
	}

	if ((hx_esd_event == length || hx_zero_event == length)
		&& (HX_HW_RESET_ACTIVATE == 0)
		&& (HX_ESD_RESET_ACTIVATE == 0)
		&& (hx_touch_data->diag_cmd == 0)
		&& (ts->in_baseline_test == 0)) {
		shaking_ret = g_core_fp.fp_ic_esd_recovery(hx_esd_event, hx_zero_event, length);

		if (shaking_ret == HX_ESD_EVENT) {
			himax_esd_hw_reset();
			ret_val = HX_ESD_EVENT;
		} else if (shaking_ret == HX_ZERO_EVENT_COUNT) {
			ret_val = HX_ZERO_EVENT_COUNT;
		} else {
			I("I2C running. Nothing to be done!\n");
			ret_val = HX_IC_RUNNING;
		}
	} else if (HX_ESD_RESET_ACTIVATE) { /* drop 1st interrupts after chip reset */
		HX_ESD_RESET_ACTIVATE = 0;
		I("[HX_ESD_RESET_ACTIVATE]:%s: Back from reset, ready to serve.\n", __func__);
		ret_val = HX_ESD_REC_OK;
	}

END_FUNCTION:
	if (g_ts_dbg != 0)
		I("%s: END, ret_val=%d! \n", __func__, ret_val);

	return ret_val;
}
#endif

static int himax_err_ctrl(struct himax_ts_data *ts, uint8_t *buf, int ts_path, int ts_status)
{
#ifdef HX_RST_PIN_FUNC
	if (HX_HW_RESET_ACTIVATE) {
		/* drop 1st interrupts after chip reset */
		HX_HW_RESET_ACTIVATE = 0;
		I("[HX_HW_RESET_ACTIVATE]:%s: Back from reset, ready to serve.\n", __func__);
		ts_status = HX_RST_OK;
		goto END_FUNCTION;
	}
#endif

	ts_status = himax_checksum_cal(ts, buf, ts_path, ts_status);
	if (ts_status == HX_CHKSUM_FAIL)
		goto CHK_FAIL;
	goto END_FUNCTION;

CHK_FAIL:
#ifdef HX_ESD_RECOVERY
	ts_status = himax_ts_event_check(ts, buf, ts_path, ts_status);
#endif


END_FUNCTION:
	if (g_ts_dbg != 0)
		I("%s: END, ts_status=%d! \n", __func__, ts_status);
	return ts_status;
}
/* end error_control*/

/* start distribute_data*/
static int himax_distribute_touch_data(uint8_t *buf, int ts_path, int ts_status)
{
	uint8_t hx_state_info_pos = hx_touch_data->touch_info_size - 3;

	g_zero_event_count = 0;

	if (g_ts_dbg != 0)
		I("%s: Entering, ts_status=%d, g_zero_event_count = %d! \n", 
		__func__, ts_status, g_zero_event_count);

	if (ts_path == HX_REPORT_COORD) {
		memcpy(hx_touch_data->hx_coord_buf, &buf[0], hx_touch_data->touch_info_size);

		if (buf[hx_state_info_pos] != 0xFF && buf[hx_state_info_pos + 1] != 0xFF) {
			memcpy(hx_touch_data->hx_state_info, &buf[hx_state_info_pos], 2);
		} else {
			memset(hx_touch_data->hx_state_info, 0x00, sizeof(hx_touch_data->hx_state_info));
		}

		if ((HX_HW_RESET_ACTIVATE)
#ifdef HX_ESD_RECOVERY
		|| (HX_ESD_RESET_ACTIVATE)
#endif
		) {
			memcpy(hx_touch_data->hx_rawdata_buf, &buf[hx_touch_data->touch_info_size], hx_touch_data->touch_all_size - hx_touch_data->touch_info_size);
		}
	} else if (ts_path == HX_REPORT_COORD_RAWDATA) {
		memcpy(hx_touch_data->hx_coord_buf, &buf[0], hx_touch_data->touch_info_size);

		if (buf[hx_state_info_pos] != 0xFF && buf[hx_state_info_pos + 1] != 0xFF) {
			memcpy(hx_touch_data->hx_state_info, &buf[hx_state_info_pos], 2);
		} else {
			memset(hx_touch_data->hx_state_info, 0x00, sizeof(hx_touch_data->hx_state_info));
		}

		memcpy(hx_touch_data->hx_rawdata_buf, &buf[hx_touch_data->touch_info_size], hx_touch_data->touch_all_size - hx_touch_data->touch_info_size);
#if defined(HX_SMART_WAKEUP)
	} else if (ts_path == HX_REPORT_SMWP_EVENT) {
		memcpy(hx_touch_data->hx_event_buf, buf, hx_touch_data->event_size);
#endif
	} else {
		E("%s, Fail Path!\n", __func__);
		ts_status = HX_PATH_FAIL;
	}

	if (g_ts_dbg != 0)
		I("%s: End, ts_status=%d! \n", __func__, ts_status);
	return ts_status;
}
/* end assign_data*/

/* start parse_report_data*/
int himax_parse_report_points(struct himax_ts_data *ts, int ts_path, int ts_status)
{
	int x = 0;
	int y = 0;
	int w = 0;
	int base = 0;
	int32_t	loop_i = 0;

	if (g_ts_dbg != 0)
		I("%s: start! \n", __func__);


	ts->old_finger = ts->pre_finger_mask;
	if (ts->hx_point_num == 0) {
		return ts_status;
	}
	ts->pre_finger_mask = 0;
	hx_touch_data->finger_num = hx_touch_data->hx_coord_buf[ts->coordInfoSize - 4] & 0x0F;
	hx_touch_data->finger_on = 1;
	AA_press = 1;

	g_target_report_data->finger_num = hx_touch_data->finger_num;
	g_target_report_data->finger_on = hx_touch_data->finger_on;

	if (g_ts_dbg != 0)
		I("%s:finger_num = 0x%2X, finger_on = %d \n", __func__, g_target_report_data->finger_num, g_target_report_data->finger_on);

	for (loop_i = 0; loop_i < ts->nFinger_support; loop_i++) {
		base = loop_i * 4;
		x = hx_touch_data->hx_coord_buf[base] << 8 | hx_touch_data->hx_coord_buf[base + 1];
		y = (hx_touch_data->hx_coord_buf[base + 2] << 8 | hx_touch_data->hx_coord_buf[base + 3]);
		w = hx_touch_data->hx_coord_buf[(ts->nFinger_support * 4) + loop_i];
		if (w < 0x10){//&& w > 0
			w = 0x05;
		}

		if (g_ts_dbg != 0)
			D("%s: now parsing[%d]:x=%d, y=%d, w=%d\n", __func__, loop_i, x, y, w);

		if (x >= 0 && x <= ts->pdata->abs_x_max && y >= 0 && y <= ts->pdata->abs_y_max) {
			hx_touch_data->finger_num--;

			g_target_report_data->x[loop_i] = x;
			g_target_report_data->y[loop_i] = y;
			g_target_report_data->w[loop_i] = w;
			g_target_report_data->finger_id[loop_i] = 1;

			/* I("%s: g_target_report_data->x[loop_i]=%d, g_target_report_data->y[loop_i]=%d, g_target_report_data->w[loop_i]=%d", */
			/* 	__func__, g_target_report_data->x[loop_i], g_target_report_data->y[loop_i], g_target_report_data->w[loop_i]); */


			if (!ts->first_pressed) {
				ts->first_pressed = 1;
				I("S1@%d, %d\n", x, y);
			}

			ts->pre_finger_data[loop_i][0] = x;
			ts->pre_finger_data[loop_i][1] = y;

			ts->pre_finger_mask = ts->pre_finger_mask + (1 << loop_i);
		} else {/* report coordinates */
			g_target_report_data->x[loop_i] = x;
			g_target_report_data->y[loop_i] = y;
			g_target_report_data->w[loop_i] = w;
			g_target_report_data->finger_id[loop_i] = 0;

			if (loop_i == 0 && ts->first_pressed == 1) {
				ts->first_pressed = 2;
				I("E1@%d, %d\n", ts->pre_finger_data[0][0], ts->pre_finger_data[0][1]);
			}
		}
	}

	if (g_ts_dbg != 0) {
		for (loop_i = 0; loop_i < 10; loop_i++) {
			D("DBG X=%d  Y=%d ID=%d\n", g_target_report_data->x[loop_i], g_target_report_data->y[loop_i], g_target_report_data->finger_id[loop_i]);
		}
		D("DBG finger number %d\n", g_target_report_data->finger_num);
	}

	if (g_ts_dbg != 0)
		I("%s: end! \n", __func__);
	return ts_status;
}

static int himax_parse_report_data(struct himax_ts_data *ts, int ts_path, int ts_status)
{

	if (g_ts_dbg != 0)
		I("%s: start now_status=%d! \n", __func__, ts_status);


	EN_NoiseFilter = (hx_touch_data->hx_coord_buf[HX_TOUCH_INFO_POINT_CNT + 2] >> 3);
	/* I("EN_NoiseFilter=%d\n", EN_NoiseFilter); */
	EN_NoiseFilter = EN_NoiseFilter & 0x01;
	/* I("EN_NoiseFilter2=%d\n", EN_NoiseFilter); */
#if defined(HX_EN_SEL_BUTTON) || defined(HX_EN_MUT_BUTTON)
	tpd_key = (hx_touch_data->hx_coord_buf[HX_TOUCH_INFO_POINT_CNT + 2] >> 4);

	/* All (VK+AA)leave */
	if (tpd_key == 0x0F) {
		tpd_key = 0x00;
	}
#endif
	p_point_num = ts->hx_point_num;
	if (hx_touch_data->hx_coord_buf[HX_TOUCH_INFO_POINT_CNT] == 0xff) {
		ts->hx_point_num = 0;
	} else {
		ts->hx_point_num = hx_touch_data->hx_coord_buf[HX_TOUCH_INFO_POINT_CNT] & 0x0f;
	}
	switch (ts_path) {
	case HX_REPORT_COORD:
		ts_status = himax_parse_report_points(ts, ts_path, ts_status);
		break;
	case HX_REPORT_COORD_RAWDATA:
		/* touch monitor rawdata */
		if (debug_data != NULL) {
			if (debug_data->fp_set_diag_cmd(ic_data, hx_touch_data))
				I("%s: coordinate dump fail and bypass with checksum err\n", __func__);
		} else {
			E("%s,There is no init set_diag_cmd\n", __func__);
		}
		ts_status = himax_parse_report_points(ts, ts_path, ts_status);
		break;
#ifdef HX_SMART_WAKEUP
	case HX_REPORT_SMWP_EVENT:
		himax_wake_event_parse(ts, ts_status);
		break;
#endif
	default:
		E("%s:Fail Path!\n", __func__);
		ts_status = HX_PATH_FAIL;
		break;
	}
	if (g_ts_dbg != 0)
		I("%s: end now_status=%d! \n", __func__, ts_status);
	return ts_status;
}

/* end parse_report_data*/

void himax_report_all_leave_event(struct himax_ts_data *ts)
{
	int loop_i = 0;

	for (loop_i = 0; loop_i < ts->nFinger_support; loop_i++) {
#ifndef	HX_PROTOCOL_A
		input_mt_slot(ts->input_dev, loop_i);
		input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);
#endif
	}
	input_report_key(ts->input_dev, BTN_TOUCH, 0);
	input_sync(ts->input_dev);
}

/* start report_data */
#if defined(HX_EN_SEL_BUTTON) || defined(HX_EN_MUT_BUTTON)
static void himax_key_report_operation(int tp_key_index, struct himax_ts_data *ts)
{
	uint16_t x_position = 0, y_position = 0;

	if (g_ts_dbg != 0)
		I("%s: Entering \n", __func__);

	if (tp_key_index != 0x00) {
		I("virtual key index =%x\n", tp_key_index);

		if (tp_key_index == 0x01) {
			vk_press = 1;
			I("back key pressed\n");

			if (ts->pdata->virtual_key) {
				if (ts->button[0].index) {
					x_position = (ts->button[0].x_range_min + ts->button[0].x_range_max) / 2;
					y_position = (ts->button[0].y_range_min + ts->button[0].y_range_max) / 2;
				}

#ifdef	HX_PROTOCOL_A
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 100);
				input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, 0);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x_position);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y_position);
				input_mt_sync(ts->input_dev);
#else
				input_mt_slot(ts->input_dev, 0);
				input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 1);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 100);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 100);
				input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 100);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x_position);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y_position);
#endif
			}	else {
				input_report_key(ts->input_dev, KEY_BACK, 1);
			}
		} else if (tp_key_index == 0x02) {
			vk_press = 1;
			I("home key pressed\n");

			if (ts->pdata->virtual_key) {
				if (ts->button[1].index) {
					x_position = (ts->button[1].x_range_min + ts->button[1].x_range_max) / 2;
					y_position = (ts->button[1].y_range_min + ts->button[1].y_range_max) / 2;
				}

#ifdef	HX_PROTOCOL_A
				input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, 0);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 100);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x_position);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y_position);
				input_mt_sync(ts->input_dev);
#else
				input_mt_slot(ts->input_dev, 0);
				input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 1);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 100);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 100);
				input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 100);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x_position);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y_position);
#endif
			} else {
				input_report_key(ts->input_dev, KEY_HOME, 1);
			}
		} else if (tp_key_index == 0x04) {
			vk_press = 1;
			I("APP_switch key pressed\n");

			if (ts->pdata->virtual_key) {
				if (ts->button[2].index) {
					x_position = (ts->button[2].x_range_min + ts->button[2].x_range_max) / 2;
					y_position = (ts->button[2].y_range_min + ts->button[2].y_range_max) / 2;
				}

#ifdef HX_PROTOCOL_A
				input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, 0);
				input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 100);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x_position);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y_position);
				input_mt_sync(ts->input_dev);
#else
				input_mt_slot(ts->input_dev, 0);
				input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 1);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 100);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 100);
				input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 100);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x_position);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y_position);
#endif
			} else {
				input_report_key(ts->input_dev, KEY_APPSELECT, 1);
			}
		}
		input_sync(ts->input_dev);
	} else { /*tp_key_index =0x00*/
		I("virtual key released\n");
		vk_press = 0;
#ifndef	HX_PROTOCOL_A
		input_mt_slot(ts->input_dev, 0);
		input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);
#else
		input_mt_sync(ts->input_dev);
#endif
		input_report_key(ts->input_dev, KEY_BACK, 0);
		input_report_key(ts->input_dev, KEY_HOME, 0);
		input_report_key(ts->input_dev, KEY_APPSELECT, 0);
#ifndef	HX_PROTOCOL_A
		input_sync(ts->input_dev);
#endif
	}
}

void himax_finger_report_key(struct himax_ts_data *ts)
{
	if (ts->hx_point_num != 0) {
		/*Touch KEY*/
		if ((tpd_key_old != 0x00) && (tpd_key == 0x00)) {
		 /* temp_x[0] = 0xFFFF;
			temp_y[0] = 0xFFFF;
			temp_x[1] = 0xFFFF;
			temp_y[1] = 0xFFFF; */
			hx_touch_data->finger_on = 0;
#ifdef HX_PROTOCOL_A
			input_report_key(ts->input_dev, BTN_TOUCH, hx_touch_data->finger_on);
#endif
			himax_key_report_operation(tpd_key, ts);
		}

#ifndef HX_PROTOCOL_A
		input_report_key(ts->input_dev, BTN_TOUCH, hx_touch_data->finger_on);
#endif
		input_sync(ts->input_dev);
	}
}

void himax_finger_leave_key(struct himax_ts_data *ts)
{
	if (tpd_key != 0x00) {
		hx_touch_data->finger_on = 1;
#ifdef HX_PROTOCOL_A
		input_report_key(ts->input_dev, BTN_TOUCH, hx_touch_data->finger_on);
#endif
		himax_key_report_operation(tpd_key, ts);
	} else if ((tpd_key_old != 0x00) && (tpd_key == 0x00)) {
		hx_touch_data->finger_on = 0;
#ifdef HX_PROTOCOL_A
		input_report_key(ts->input_dev, BTN_TOUCH, hx_touch_data->finger_on);
#endif
		himax_key_report_operation(tpd_key, ts);
	}

#ifndef HX_PROTOCOL_A
	input_report_key(ts->input_dev, BTN_TOUCH, hx_touch_data->finger_on);
#endif
	input_sync(ts->input_dev);
}

static void himax_report_key(struct himax_ts_data *ts)
{
	if (ts->hx_point_num != 0) { /* Touch KEY */
		himax_finger_report_key(ts);
	} else { /* Key */
		himax_finger_leave_key(ts);
	}

	tpd_key_old = tpd_key;
	Last_EN_NoiseFilter = EN_NoiseFilter;
}
#endif

/* start report_point*/
static void himax_finger_report(struct himax_ts_data *ts)
{
	int i = 0;
	bool valid = false;
	if (g_ts_dbg != 0) {
		I("%s:start\n", __func__);
		I("hx_touch_data->finger_num=%d\n", hx_touch_data->finger_num);
	}
	memset(corner_info, 0, sizeof(corner_info));
	for (i = 0; i < ts->nFinger_support; i++) {
		if (g_target_report_data->x[i] >= 0 && g_target_report_data->x[i] <= ts->pdata->abs_x_max && g_target_report_data->y[i] >= 0 && g_target_report_data->y[i] <= ts->pdata->abs_y_max)
			valid = true;
		else
			valid = false;
		if (g_ts_dbg != 0)
			I("valid=%d\n", valid);
		if (valid) {
			if (g_ts_dbg != 0)
				I("g_target_report_data->x[i]=%d, g_target_report_data->y[i]=%d, g_target_report_data->w[i]=%d\n", g_target_report_data->x[i], g_target_report_data->y[i], g_target_report_data->w[i]);
#ifndef	HX_PROTOCOL_A
			input_mt_slot(ts->input_dev, i);
#endif
			input_report_key(ts->input_dev, BTN_TOUCH, g_target_report_data->finger_on);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_target_report_data->w[i]);
			//input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
#ifndef	HX_PROTOCOL_A
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_target_report_data->w[i]);
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, g_target_report_data->w[i]);
#else
			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
#endif
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_target_report_data->x[i]);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_target_report_data->y[i]);
#ifndef	HX_PROTOCOL_A
			ts->last_slot = i;
			input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 1);
#ifdef HX_EDGE_LIMIT
			points_info[i].x = g_target_report_data->x[i];
			points_info[i].y = g_target_report_data->y[i];
			points_info[i].type = AREA_NORMAL;
			corner_point_process(ts, corner_info, points_info, i);
#endif
#else
			input_mt_sync(ts->input_dev);
#endif

		} else {
			input_mt_slot(ts->input_dev, i);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
			input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);
		}
	}

	input_report_key(ts->input_dev, BTN_TOUCH, g_target_report_data->finger_on);
	input_sync(ts->input_dev);

	if (g_ts_dbg != 0)
		I("%s:end\n", __func__);
}

static void himax_finger_leave(struct himax_ts_data *ts)
{
	int32_t loop_i = 0;

	if (g_ts_dbg != 0)
		I("%s: start! \n", __func__);
#if defined(HX_PALM_REPORT)
	if (himax_palm_detect(hx_touch_data->hx_coord_buf) == PALM_REPORT) {
		I(" %s HX_PALM_REPORT KEY power event press\n", __func__);
		input_report_key(ts->input_dev, KEY_POWER, 1);
		input_sync(ts->input_dev);
		msleep(100);

		I(" %s HX_PALM_REPORT KEY power event release\n", __func__);
		input_report_key(ts->input_dev, KEY_POWER, 0);
		input_sync(ts->input_dev);
		return;
	}
#endif

	hx_touch_data->finger_on = 0;
	AA_press = 0;

#ifdef HX_PROTOCOL_A
	input_mt_sync(ts->input_dev);
#endif
	for (loop_i = 0; loop_i < ts->nFinger_support; loop_i++) {
		//if (((ts->old_finger >> loop_i) & 1) == 1) {
		if (((ts->pre_finger_mask >> loop_i) & 1) == 1) {
			input_mt_slot(ts->input_dev, loop_i);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
			input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);
		}
	}

	if (ts->pre_finger_mask > 0) {
		ts->pre_finger_mask = 0;
	}

	if (ts->first_pressed == 1) {
		ts->first_pressed = 2;
		I("E1@%d, %d\n", ts->pre_finger_data[0][0], ts->pre_finger_data[0][1]);
	}

	/* if (ts->debug_log_level & BIT(1)) */
	/* 	himax_log_touch_event(x, y, w, loop_i, EN_NoiseFilter, HX_FINGER_LEAVE); */

	input_report_key(ts->input_dev, BTN_TOUCH, hx_touch_data->finger_on);
	input_sync(ts->input_dev);

	if (g_ts_dbg != 0)
		I("%s: end! \n", __func__);
	return;
}

static void himax_report_points(struct himax_ts_data *ts)
{
	if (g_ts_dbg != 0)
		I("%s: start! \n", __func__);

	if (ts->hx_point_num != 0) {
		himax_finger_report(ts);
	} else {
		himax_finger_leave(ts);
	}

	Last_EN_NoiseFilter = EN_NoiseFilter;

	if (g_ts_dbg != 0)
		I("%s: end! \n", __func__);
}
/* end report_points*/

int himax_report_data(struct himax_ts_data *ts, int ts_path, int ts_status)
{
	if (g_ts_dbg != 0)
		I("%s: Entering, ts_status=%d! \n", __func__, ts_status);

	if (ts_path == HX_REPORT_COORD || ts_path == HX_REPORT_COORD_RAWDATA) {
		/*if (hx_touch_data->hx_coord_buf[HX_TOUCH_INFO_POINT_CNT] == 0xff) {
			ts->hx_point_num = 0;
		} else {
			ts->hx_point_num = hx_touch_data->hx_coord_buf[HX_TOUCH_INFO_POINT_CNT] & 0x0f;
		}*/

		/* Touch Point information */
		himax_report_points(ts);

#if defined(HX_EN_SEL_BUTTON) || defined(HX_EN_MUT_BUTTON)
		/*report key(question mark)*/
		if (tpd_key && tpd_key_old) {
			himax_report_key(ts);
		}
#endif
#ifdef HX_SMART_WAKEUP
	} else if (ts_path == HX_REPORT_SMWP_EVENT) {
		//wake_lock_timeout(&ts->ts_SMWP_wake_lock, TS_WAKE_LOCK_TIMEOUT);
		pm_wakeup_event(&ts->input_dev->dev, 5000);
		himax_wake_event_report();
#endif
	} else {
		E("%s:Fail Path!\n", __func__);
		ts_status = HX_PATH_FAIL;
	}

	if (g_ts_dbg != 0)
		I("%s: END, ts_status=%d! \n", __func__, ts_status);
	return ts_status;
}
/* end report_data */

static int himax_ts_operation(struct himax_ts_data *ts, int ts_path, int ts_status)
{
	uint8_t hw_reset_check[2];
	uint8_t buf[128];

	memset(buf, 0x00, sizeof(buf));
	memset(hw_reset_check, 0x00, sizeof(hw_reset_check));

	ts_status = himax_touch_get(ts, buf, ts_path, ts_status);
	if (ts_status == HX_TS_GET_DATA_FAIL)
		goto END_FUNCTION;

	ts_status = himax_err_ctrl(ts, buf, ts_path, ts_status);
	if (ts_status == HX_REPORT_DATA || ts_status == HX_TS_NORMAL_END) {
		ts_status = himax_distribute_touch_data(buf, ts_path, ts_status);
		ts_status = himax_parse_report_data(ts, ts_path, ts_status);
	} else {
		goto END_FUNCTION;
	}
	ts_status = himax_report_data(ts, ts_path, ts_status);


END_FUNCTION:
	return ts_status;
}

void himax_ts_work(struct himax_ts_data *ts)
{

	int ts_status = HX_TS_NORMAL_END;
	int ts_path = 0;

	if (debug_data != NULL)
		debug_data->fp_ts_dbg_func(ts, HX_FINGER_ON);

#if defined(HX_USB_DETECT_GLOBAL)
	himax_cable_detect_func(false);
#endif
	ts_path = himax_ts_work_status(ts);
	switch (ts_path) {
	case HX_REPORT_COORD:
		ts_status = himax_ts_operation(ts, ts_path, ts_status);
		break;
	case HX_REPORT_SMWP_EVENT:
		//msleep(100);
		ts_status = himax_ts_operation(ts, ts_path, ts_status);
		break;
	case HX_REPORT_COORD_RAWDATA:
		ts_status = himax_ts_operation(ts, ts_path, ts_status);
		break;
	default:
		E("%s:Path Fault! value=%d\n", __func__, ts_path);
		goto END_FUNCTION;
		break;
	}

	if (ts_status == HX_TS_GET_DATA_FAIL)
		goto GET_TOUCH_FAIL;
	else
		goto END_FUNCTION;

GET_TOUCH_FAIL:
	I("%s: Now reset the Touch chip.\n", __func__);
#ifdef HX_RST_PIN_FUNC
	g_core_fp.fp_ic_reset(false, true);
#endif
END_FUNCTION:
	if (debug_data != NULL)
		debug_data->fp_ts_dbg_func(ts, HX_FINGER_LEAVE);
	enable_irq(private_ts->hx_irq);
	return;
}
/*end ts_work*/
enum hrtimer_restart himax_ts_timer_func(struct hrtimer *timer)
{
	struct himax_ts_data *ts;
	ts = container_of(timer, struct himax_ts_data, timer);
	queue_work(ts->himax_wq, &ts->work);
	hrtimer_start(&ts->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

#if defined(HX_USB_DETECT_CALLBACK)
static void himax_cable_tp_status_handler_func(int connect_status)
{
	struct himax_ts_data *ts;
	I("Touch: cable change to %d\n", connect_status);
	ts = private_ts;

	if (ts->cable_config) {
		if (!atomic_read(&ts->suspend_mode)) {
			if (connect_status != ts->usb_connected) {
				if (connect_status) {
					ts->cable_config[1] = 0x01;
					ts->usb_connected = 0x01;
				} else {
					ts->cable_config[1] = 0x00;
					ts->usb_connected = 0x00;
				}

				himax_bus_master_write(ts->cable_config,
										sizeof(ts->cable_config), HIMAX_I2C_RETRY_TIMES);
				I("%s: Cable status change: 0x%2.2X\n", __func__, ts->cable_config[1]);
			} else {
					I("%s: Cable status is the same as previous one, ignore.\n", __func__);
			}
		} else {
			if (connect_status) {
				ts->usb_connected = 0x01;
			} else {
				ts->usb_connected = 0x00;
			}

			I("%s: Cable status remembered: 0x%2.2X\n", __func__, ts->usb_connected);
		}
	}
}

static struct t_cable_status_notifier himax_cable_status_handler = {
	.name = "usb_tp_connected",
	.func = himax_cable_tp_status_handler_func,
};

#endif

#ifdef HX_AUTO_UPDATE_FW
static void himax_update_register(struct work_struct *work)
{
	I(" %s in\n", __func__);

	if (i_get_FW() != 0)
		return ;

	if (g_auto_update_flag == true) {
		I("Update FW Directly");
		goto UPDATE_FW;
	}

	if (himax_auto_update_check() != 0) {
		return ;
	}

UPDATE_FW:
	if (i_update_FW() <= 0) {
		I("Auto update FW fail!\n");
	}	else {
		I("It have Updated\n");
	}
}
#endif


#ifdef CONFIG_FB
static void himax_fb_register(struct work_struct *work)
{
	int ret = 0;
	struct himax_ts_data *ts = container_of(work, struct himax_ts_data, work_att.work);
	I(" %s in\n", __func__);
	ts->fb_notif.notifier_call = fb_notifier_callback;
	ret = fb_register_client(&ts->fb_notif);

	if (ret) {
		E(" Unable to register fb_notifier: %d\n", ret);
	}
}
#endif
static int himax_notifie_update_fw(struct notifier_block *self,
								unsigned long event, void *data)
{
	I(" %s in\n", __func__);
	I(" event = %ld \n", event);
	if (event == 1) {
		lcd_resume_load_himax_fw();
	}

	return 0;
}

int himax_chip_common_init(void)
{

	int ret = 0, err = 0;
	struct himax_ts_data *ts = private_ts;
	struct himax_i2c_platform_data *pdata;

	//I("PDATA START\n");
	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);

	if (pdata == NULL) { /*Allocate Platform data space*/
		E("PDATA is NULL\n");
		err = -ENOMEM;
		goto err_dt_platform_data_fail;
	}

	//I("ic_data START\n");
	ic_data = kzalloc(sizeof(*ic_data), GFP_KERNEL);
	if (ic_data == NULL) { /*Allocate IC data space*/
		E("IC DATA is NULL\n");
		err = -ENOMEM;
		goto err_dt_ic_data_fail;
	}

	/* allocate report data */
	hx_touch_data = kzalloc(sizeof(struct himax_report_data), GFP_KERNEL);
	if (hx_touch_data == NULL) {
		err = -ENOMEM;
		goto err_alloc_touch_data_failed;
	}

	if (himax_parse_dt(ts, pdata) < 0) {
		I(" pdata is NULL for DT\n");
		goto err_alloc_dt_pdata_failed;
	}
		pdata->abs_x_min = 0;
		pdata->abs_x_max = 720;
		pdata->abs_y_min = 0;
		pdata->abs_y_max = 1520;
		pdata->screenHeight = 1520;
		pdata->screenWidth = 720;
		I(" DT-%s:panel-coords = %d, %d, %d, %d\n", __func__, pdata->abs_x_min,
			pdata->abs_x_max, pdata->abs_y_min, pdata->abs_y_max);
		I(" DT-%s:display-coords = %d, %d\n", __func__, pdata->screenWidth,
			pdata->screenHeight);

#ifdef HX_RST_PIN_FUNC
	ts->rst_gpio = pdata->gpio_reset;
#endif
	himax_gpio_power_config(pdata);
#ifndef CONFIG_OF

	if (pdata->power) {
		ret = pdata->power(1);

		if (ret < 0) {
			E("%s: power on failed\n", __func__);
			goto err_power_failed;
		}
	}

#endif

	if (g_core_fp.fp_chip_detect != NULL) {
		if (g_core_fp.fp_chip_detect() != false) {
			if (g_core_fp.fp_chip_init != NULL) {
				g_core_fp.fp_chip_init();
			} else {
				E("%s: function point of chip_init is NULL!\n", __func__);
				goto error_ic_detect_failed;
			}
		} else {
			E("%s: chip detect failed!\n", __func__);
			goto error_ic_detect_failed;
		}
	} else {
		E("%s: function point of chip_detect is NULL!\n", __func__);
		goto error_ic_detect_failed;
	}


	if (pdata->virtual_key) {
		ts->button = pdata->virtual_key;
	}

#ifdef HX_AUTO_UPDATE_FW
	g_auto_update_flag = (!g_core_fp.fp_calculateChecksum(false));
	g_auto_update_flag |= g_core_fp.fp_flash_lastdata_check();
	if (g_auto_update_flag)
		goto FW_force_upgrade;
#endif
#ifndef HX_ZERO_FLASH
	g_core_fp.fp_read_FW_ver();
#endif

#ifdef HX_AUTO_UPDATE_FW
FW_force_upgrade:
	ts->himax_update_wq = create_singlethread_workqueue("HMX_update_reuqest");
	if (!ts->himax_update_wq) {
		E(" allocate syn_update_wq failed\n");
		err = -ENOMEM;
		goto err_update_wq_failed;
	}
	INIT_DELAYED_WORK(&ts->work_update, himax_update_register);
	queue_delayed_work(ts->himax_update_wq, &ts->work_update, msecs_to_jiffies(2000));
#endif
#ifdef HX_ZERO_FLASH
	g_auto_update_flag = true;
	ts->himax_0f_update_wq = create_singlethread_workqueue("HMX_0f_update_reuqest");
	INIT_DELAYED_WORK(&ts->work_0f_update, g_core_fp.fp_0f_operation);
	queue_delayed_work(ts->himax_0f_update_wq, &ts->work_0f_update, msecs_to_jiffies(2000));
#endif
	INIT_WORK(&ts->resume_update_fw, g_core_fp.update_fw_func);

	/*Himax Power On and Load Config*/
	//if (himax_loadSensorConfig(pdata)) {
	//	E("%s: Load Sesnsor configuration failed, unload driver.\n", __func__);
	//	goto err_detect_failed;
	//}

	g_core_fp.fp_power_on_init();
	calculate_point_number();

#ifdef CONFIG_OF
	ts->power = pdata->power;
#endif
	ts->pdata = pdata;
	ts->x_channel = ic_data->HX_RX_NUM;
	ts->y_channel = ic_data->HX_TX_NUM;
	ts->nFinger_support = ic_data->HX_MAX_PT;
	/*calculate the i2c data size*/
	calcDataSize(ts->nFinger_support);
	//I("%s: calcDataSize complete\n", __func__);
#ifdef CONFIG_OF
	ts->pdata->abs_pressure_min        = 0;
	ts->pdata->abs_pressure_max        = 200;
	ts->pdata->abs_width_min           = 0;
	ts->pdata->abs_width_max           = 200;
	pdata->cable_config[0]             = 0xF0;
	pdata->cable_config[1]             = 0x00;
#endif
	ts->suspended                      = false;
#if defined(HX_USB_DETECT_CALLBACK) || defined(HX_USB_DETECT_GLOBAL)
	ts->usb_connected = 0x00;
	ts->cable_config = pdata->cable_config;
#endif
#ifdef	HX_PROTOCOL_A
	ts->protocol_type = PROTOCOL_TYPE_A;
#else
	ts->protocol_type = PROTOCOL_TYPE_B;
#endif
	I("%s: Use Protocol Type %c\n", __func__,
	  ts->protocol_type == PROTOCOL_TYPE_A ? 'A' : 'B');
	ret = himax_input_register(ts);

	if (ret) {
		E("%s: Unable to register %s input device\n",
		  __func__, ts->input_dev->name);
		goto err_input_register_device_failed;
	}

#ifdef CONFIG_FB
	ts->himax_att_wq = create_singlethread_workqueue("HMX_ATT_reuqest");

	if (!ts->himax_att_wq) {
		E(" allocate syn_att_wq failed\n");
		err = -ENOMEM;
		goto err_get_intr_bit_failed;
	}

	INIT_DELAYED_WORK(&ts->work_att, himax_fb_register);
	queue_delayed_work(ts->himax_att_wq, &ts->work_att, msecs_to_jiffies(5000));
#endif
	ts->update_fw_notif.notifier_call = himax_notifie_update_fw;
 	ret = update_tpfw_register_client(&ts->update_fw_notif);

#ifdef HX_SMART_WAKEUP
	ts->SMWP_enable = 0;
	//wake_lock_init(&ts->ts_SMWP_wake_lock, WAKE_LOCK_SUSPEND, HIMAX_common_NAME);
	device_init_wakeup(&ts->input_dev->dev, 1);
#endif
#ifdef HX_HIGH_SENSE
	ts->HSEN_enable = 0;
#endif

	/*touch data init*/
	err = himax_report_data_init();

	if (err) {
		goto err_report_data_init_failed;
	}

	if (himax_common_proc_init()) {
		E(" %s: himax_common proc_init failed!\n", __func__);
		goto err_creat_proc_file_failed;
	}

#if defined(HX_USB_DETECT_CALLBACK)

	if (ts->cable_config)	{
		cable_detect_register_notifier(&himax_cable_status_handler);
	}

#endif
	err = himax_ts_register_interrupt();

	if (err) {
		goto err_register_interrupt_failed;
	}

#ifdef CONFIG_TOUCHSCREEN_HIMAX_DEBUG
	if (himax_debug_init())
		E(" %s: debug initial failed!\n", __func__);
#endif

	INIT_WORK(&ts->himax_lcd_update_workqueue, g_core_fp.fp_resume_work/*himax_chip_common_resume_workqueue*/);
	himax_tp = 1;
	spin_lock_init(&ts->irq_lock);
#if defined(HX_AUTO_UPDATE_FW) || defined(HX_ZERO_FLASH)

	if (g_auto_update_flag) {
		himax_int_enable(0);
	}

#endif
	if ((ts->boot_mode == 3 || ts->boot_mode == 4 || ts->boot_mode == 5)) {
		disable_irq_nosync(private_ts->hx_irq);
		I(" %s: FTM mode disable irq\n", __func__);
	}
	return 0;
err_register_interrupt_failed:
	remove_proc_entry(HIMAX_PROC_TOUCH_FOLDER, NULL);
err_creat_proc_file_failed:
err_report_data_init_failed:
#ifdef HX_SMART_WAKEUP
	//wake_lock_destroy(&ts->ts_SMWP_wake_lock);
#endif
#ifdef CONFIG_FB
	cancel_delayed_work_sync(&ts->work_att);
	destroy_workqueue(ts->himax_att_wq);
err_get_intr_bit_failed:
#endif
err_input_register_device_failed:
	input_free_device(ts->input_dev);
//err_detect_failed:

#ifdef HX_AUTO_UPDATE_FW
	if (g_auto_update_flag) {
		cancel_delayed_work_sync(&ts->work_update);
		destroy_workqueue(ts->himax_update_wq);
	}
err_update_wq_failed:
#endif
error_ic_detect_failed:
	if (gpio_is_valid(pdata->gpio_irq)) {
		gpio_free(pdata->gpio_irq);
	}
#ifdef HX_RST_PIN_FUNC
	if (gpio_is_valid(pdata->gpio_reset))	{
		gpio_free(pdata->gpio_reset);
	}
#endif
#ifndef CONFIG_OF
err_power_failed:
#endif
err_alloc_dt_pdata_failed:
	kfree(hx_touch_data);
err_alloc_touch_data_failed:
	kfree(ic_data);
err_dt_ic_data_fail:
	kfree(pdata);
err_dt_platform_data_fail:
	kfree(ts);
	probe_fail_flag = 1;
	return err;
}

void himax_chip_common_deinit(void)
{
	struct himax_ts_data *ts = private_ts;

#ifdef CONFIG_TOUCHSCREEN_HIMAX_DEBUG
	himax_debug_remove();
#endif

	remove_proc_entry(HIMAX_PROC_TOUCH_FOLDER, NULL);
	himax_common_proc_deinit();

	if (!ts->use_irq) {
		hrtimer_cancel(&ts->timer);
		destroy_workqueue(ts->himax_wq);
	}

#ifdef HX_SMART_WAKEUP
	//wake_lock_destroy(&ts->ts_SMWP_wake_lock);
#endif
#ifdef CONFIG_FB
	if (fb_unregister_client(&ts->fb_notif))
		E("Error occurred while unregistering fb_notifier.\n");
	cancel_delayed_work_sync(&ts->work_att);
	destroy_workqueue(ts->himax_att_wq);
#endif
	input_free_device(ts->input_dev);
#ifdef HX_ZERO_FLASH
	cancel_delayed_work_sync(&ts->work_0f_update);
	destroy_workqueue(ts->himax_0f_update_wq);
#endif
#ifdef HX_AUTO_UPDATE_FW
	cancel_delayed_work_sync(&ts->work_update);
	destroy_workqueue(ts->himax_update_wq);
#endif
	if (gpio_is_valid(ts->pdata->gpio_irq))
		gpio_free(ts->pdata->gpio_irq);
#ifdef HX_RST_PIN_FUNC
	if (gpio_is_valid(private_ts->rst_gpio)) {
		printk("himax ctp reset value = %d\n",gpio_get_value(private_ts->rst_gpio));
		if ( gpio_get_value(private_ts->rst_gpio) == 1 ) {
			gpio_set_value(private_ts->rst_gpio, 0);
			printk("himax ctp reset value1 = %d\n",gpio_get_value(private_ts->rst_gpio));
	
		}
	}

	if (gpio_is_valid(ts->pdata->gpio_reset))
		gpio_free(ts->pdata->gpio_reset);
	
#endif

	kfree(hx_touch_data);
	kfree(ic_data);
	kfree(ts->pdata);
	kfree(ts);
	probe_fail_flag = 0;
	ts->SMWP_enable = 0;
	g_gesture = 0;
	himax_tp = 0;
	return;
}

int himax_chip_common_suspend(struct himax_ts_data *ts)
{
	int ret;
    //int lcd_flags = 0;
	//int pwr = 0;
	if (ts->suspended) {
		I("%s: Already suspended. Skipped. \n", __func__);
		return 0;
	} else {
		ts->suspended = true;
		I("%s: enter \n", __func__);
	}

	if (debug_data != NULL && debug_data->flash_dump_going == true) {
		I("[himax] %s: Flash dump is going, reject suspend\n", __func__);
		return 0;
	}

	HX_HW_RESET_ACTIVATE = 0;

#if defined(HX_SMART_WAKEUP) || defined(HX_HIGH_SENSE) || defined(HX_USB_DETECT_GLOBAL)
#ifndef HX_RESUME_SEND_CMD
	g_core_fp.fp_resend_cmd_func(ts->suspended);
#endif
#endif

#ifdef HX_SMART_WAKEUP

	if (ts->SMWP_enable) {
		/*if (gpio_is_valid(private_ts->lcd_reset_gpio)) {
            lcd_flags = gpio_request(private_ts->lcd_reset_gpio, "NVT-lcd-rst");
			printk("ts->lcd_reset_gpio = %d\n",private_ts->lcd_reset_gpio);
            if (lcd_flags) {
	            E("Failed to request NVT-lcd-rst GPIO\n");
		    }
			printk("gpio_get_value(ts->lcd_reset_gpio) gesture  = %d\n",gpio_get_value(private_ts->lcd_reset_gpio));
            gpio_direction_output(private_ts->lcd_reset_gpio, 1);
			printk("gpio_get_value(ts->lcd_reset_gpio) gesture  = %d\n",gpio_get_value(private_ts->lcd_reset_gpio));
	    }*/
		irq_set_irq_wake(private_ts->hx_irq , 1);
		atomic_set(&ts->suspend_mode, 1);
		ts->pre_finger_mask = 0;
		FAKE_POWER_KEY_SEND = false;
		I("[himax] %s: SMART_WAKEUP enable, reject suspend\n", __func__);
		return 0;
	}
	  else {
	  /*if (gpio_is_valid(private_ts->lcd_reset_gpio)) {
			printk("gpio_get_value(ts->lcd_reset_gpio)1  = %d\n",gpio_get_value(private_ts->lcd_reset_gpio));
            lcd_flags = gpio_request(private_ts->lcd_reset_gpio, "NVT-lcd-rst");
			printk("ts->lcd_reset_gpio = %d\n",private_ts->lcd_reset_gpio);
            if (lcd_flags) {
	            E("himax Failed to request lcd-rst GPIO\n");
		    }
			printk("gpio_get_value(ts->lcd_reset_gpio)2  = %d\n",gpio_get_value(private_ts->lcd_reset_gpio));
            gpio_direction_output(private_ts->lcd_reset_gpio, 0);
			msleep(20);
			gpio_free(private_ts->lcd_reset_gpio);
			printk("gpio_get_value(ts->lcd_reset_gpio)3  = %d\n",gpio_get_value(private_ts->lcd_reset_gpio));
	    }*/
	  #if 0
	  if (gpio_is_valid(private_ts->rst_gpio)) {
		  printk("himax ctp reset value = %d\n",gpio_get_value(private_ts->rst_gpio));
		  if ( gpio_get_value(private_ts->rst_gpio) == 1 ) {
			  gpio_set_value(private_ts->rst_gpio, 0);
			  printk("himax ctp reset value1 = %d\n",gpio_get_value(private_ts->rst_gpio));

		  }
	  }
	  #endif
	 // pwr = HX_power_on(ts,false);
		//if(pwr != 0) {
			//printk("%s:power off fail\n",__func__);
		//}
	}

#endif
	himax_int_enable(0);
	g_core_fp.fp_suspend_ic_action();

	if (!ts->use_irq) {
		ret = cancel_work_sync(&ts->work);

		if (ret) {
			himax_int_enable(1);
		}
	}

	/*ts->first_pressed = 0;*/
	atomic_set(&ts->suspend_mode, 1);
	ts->pre_finger_mask = 0;

	if (ts->pdata->powerOff3V3 && ts->pdata->power) {
		ts->pdata->power(0);
	}

	I("%s: END \n", __func__);
	return 0;
}

int himax_chip_common_resume(struct himax_ts_data *ts)
{
#ifdef ODM_WT_EDIT
//Tianchen.Zhao@ODM_RH.TP Porting

//#ifndef HX_SMART_WAKEUP
//#if defined(HX_RST_PIN_FUNC) && defined(HX_RESUME_HW_RESET) && defined(HX_ZERO_FLASH)
	//int result = 0;
//#endif
#endif
    //int pwr = 0;
    //int gpio = 0;
	I("%s: enter \n", __func__);

#if 0// do resume in workqueue
	if (ts->suspended == false) {
		I("%s: It had entered resume, skip this step \n", __func__);
		return 0;
	} else {
		ts->suspended = false;
	}

	atomic_set(&ts->suspend_mode, 0);
#ifdef HX_SMART_WAKEUP
if ( !(ts->SMWP_enable)) {
	/*gpio = gpio_get_value(private_ts->lcd_reset_gpio);
	printk("ts->lcd_reset_gpio = %d",gpio);
    printk("gpio_get_value(ts->lcd_reset_gpio) resume  = %d\n",gpio_get_value(private_ts->lcd_reset_gpio));
    if (gpio_is_valid(private_ts->lcd_reset_gpio)) {
	    gpio_set_value(private_ts->lcd_reset_gpio, 1);
	    //gpio_free(private_ts->lcd_reset_gpio);
    }
    pwr = HX_power_on(ts,true);
	if(pwr != 0) {
		E("%s:power on fail\n",__func__);
		}*/
	if (gpio_is_valid(private_ts->rst_gpio)) {
		printk("himax ctp reset value = %d\n",gpio_get_value(private_ts->rst_gpio));
		if ( gpio_get_value(private_ts->rst_gpio) == 0) {
			gpio_set_value(private_ts->rst_gpio, 1);
			printk("himax ctp reset value1 = %d\n",gpio_get_value(private_ts->rst_gpio));

		}
	}
}
//ts->psensor_stus = 0;
#endif

	if (ts->pdata->powerOff3V3 && ts->pdata->power) {
		ts->pdata->power(1);
	}

#if defined(HX_SMART_WAKEUP) || defined(HX_HIGH_SENSE) || defined(HX_USB_DETECT_GLOBAL)
	g_core_fp.fp_resend_cmd_func(ts->suspended);
	
#elif defined(HX_RST_PIN_FUNC) && defined(HX_RESUME_HW_RESET)
	g_core_fp.fp_ic_reset(false, false);
#endif
#if 0
#ifdef HX_ZERO_FLASH
	I("It will update fw after esd event in zero flash mode!\n");
#ifdef ODM_WT_EDIT
//Tianchen.Zhao@ODM_RH.TP Porting
	result = g_core_fp.fp_0f_operation_dirly();
	if(result) {
		E("Something is wrong! Skip Update with zero flash! %s: END \n",__func__);
		return 0;
	}
#endif
	g_core_fp.fp_reload_disable(0);
	g_core_fp.fp_sense_on(0x00);
#endif

	himax_report_all_leave_event(ts);

	g_core_fp.fp_resume_ic_action();

	himax_int_enable(1);
#endif
	schedule_work(&ts->resume_update_fw);
#endif// do resume in workqueue

	I("%s: END \n", __func__);
	return 0;
}

void lcd_resume_load_himax_fw(void)
{
	schedule_work(&private_ts->himax_lcd_update_workqueue);
	return;
}


