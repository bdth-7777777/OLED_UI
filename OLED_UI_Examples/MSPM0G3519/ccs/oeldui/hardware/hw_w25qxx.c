#include "hw_w25qxx.h"
#include "hw_delay.h"
uint8_t spi_read_write_byte(uint8_t dat)
{
        uint8_t data = 0;

        //发送数据
        DL_SPI_transmitData8(SPI_FLASH_INST,dat);
        //等待SPI总线空闲
        while(DL_SPI_isBusy(SPI_FLASH_INST));
        //接收数据
        data = DL_SPI_receiveData8(SPI_FLASH_INST);
        //等待SPI总线空闲
        while(DL_SPI_isBusy(SPI_FLASH_INST));

        return data;
}

//读取芯片ID
//返回值如下:
//0XEF13,表示芯片型号为W25Q80
//0XEF14,表示芯片型号为W25Q16
//0XEF15,表示芯片型号为W25Q32
//0XEF16,表示芯片型号为W25Q64
//0XEF17,表示芯片型号为W25Q128
//读取设备ID
uint16_t W25Q128_readID(void)
{
    volatile uint16_t temp = 0;
    //将CS端拉低为低电平
    SPI_CS(0);
    //发送指令90h
    spi_read_write_byte(0x90);//发送读取ID命令
    //发送地址  000000H
    spi_read_write_byte(0x00);
    spi_read_write_byte(0x00);
    spi_read_write_byte(0x00);

    //接收数据
    //接收制造商ID
    temp |= spi_read_write_byte(0xFF)<<8;
    //接收设备ID
    temp |= spi_read_write_byte(0xFF);
    //恢复CS端为高电平
    SPI_CS(1);
    //返回ID
    return temp;
}

//发送写使能
void W25Q128_write_enable(void)
{
    //拉低CS端为低电平
    SPI_CS(0);
    //发送指令06h
    spi_read_write_byte(0x06);
    //拉高CS端为高电平
    SPI_CS(1);
}

//等待忙检测
void W25Q128_wait_busy(void)
{
	volatile unsigned char byte = 0;
	do
	 {
			//拉低CS端为低电平
			SPI_CS(0);
			//发送指令05h
			spi_read_write_byte(0x05);
			//接收状态寄存器值
			byte = spi_read_write_byte(0Xff);
			//恢复CS端为高电平
			SPI_CS(1);
	 //判断BUSY位是否为1 如果为1说明在忙，重新读写BUSY位直到为0
	 }while( ( byte & 0x01 ) == 1 );
}

/**********************************************************
 * 函 数 名 称：W25Q128_erase_sector
 * 函 数 功 能：擦除一个扇区 4K内容
 * 传 入 参 数：addr=擦除的扇区号
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：addr=擦除的扇区号，范围=0~15
**********************************************************/
void W25Q128_erase_sector(uint32_t addr)
{
    volatile uint32_t addr_temp = 0;
	//计算扇区号，一个扇区4KB=4096
	addr_temp = addr * 4096;
	W25Q128_write_enable();  //写使能
	W25Q128_wait_busy();     //判断忙，如果忙则一直等待
	//拉低CS端为低电平
	SPI_CS(0);
	//发送指令20h
	spi_read_write_byte(0x20);
	//发送24位扇区地址的高8位
	spi_read_write_byte((uint8_t)((addr_temp)>>16));
	//发送24位扇区地址的中8位
	spi_read_write_byte((uint8_t)((addr_temp)>>8));
	//发送24位扇区地址的低8位
	spi_read_write_byte((uint8_t)addr_temp);
	//恢复CS端为高电平
	SPI_CS(1);
	//等待擦除完成
	W25Q128_wait_busy();
}

//擦除整片芯片FLASH
char W25Q128_erase_sector_all(void)
{
	W25Q128_write_enable();//写使能
	SPI_CS(0);
	spi_read_write_byte(0xC7);
	SPI_CS(1);
	
    W25Q128_wait_busy();//如果擦除未完成，在忙
	
    //擦除成功	
	return 0;	
}
/**********************************************************
 * 函 数 名 称：W25Q128_write
 * 函 数 功 能：写数据到W25Q128进行保存
 * 传 入 参 数：buffer=写入的数据内容        addr=写入地址        numbyte=写入数据的长度
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
**********************************************************/
void W25Q128_write(uint8_t* buffer, uint32_t addr, uint16_t numbyte)
{
    unsigned int i = 0;
    //擦除扇区数据
    W25Q128_erase_sector(addr/4096);
    //写使能
    W25Q128_write_enable();
    //忙检测
    W25Q128_wait_busy();
    //写入数据
    //拉低CS端为低电平
    SPI_CS(0);
    //发送指令02h
    spi_read_write_byte(0x02);
    //发送写入的24位地址中的高8位
    spi_read_write_byte((uint8_t)(addr>>16));
    //发送写入的24位地址中的中8位
    spi_read_write_byte((uint8_t)(addr>>8));
    //发送写入的24位地址中的低8位
    spi_read_write_byte((uint8_t)addr);
    //根据写入的字节长度连续写入数据buffer
    for(i=0;i<numbyte;i++)
    {
        spi_read_write_byte(buffer[i]);
    }
    //恢复CS端为高电平
    SPI_CS(1);
    //忙检测
    W25Q128_wait_busy();
}

/**********************************************************
 * 函 数 名 称：W25Q128_read
 * 函 数 功 能：读取W25Q128的数据
 * 传 入 参 数：buffer=读出数据的保存地址  read_addr=读取地址   read_length=读去长度
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
**********************************************************/
void W25Q128_read(uint8_t* buffer,uint32_t read_addr,uint16_t read_length)
{
	uint16_t i;
	//拉低CS端为低电平
	SPI_CS(0);
	//发送指令03h
	spi_read_write_byte(0x03);
	//发送24位读取数据地址的高8位
	spi_read_write_byte((uint8_t)((read_addr)>>16));
	//发送24位读取数据地址的中8位
	spi_read_write_byte((uint8_t)((read_addr)>>8));
	//发送24位读取数据地址的低8位
	spi_read_write_byte((uint8_t)read_addr);
	//根据读取长度读取出地址保存到buffer中
	for(i=0;i<read_length;i++)
	{
		buffer[i]= spi_read_write_byte(0XFF);
	}
	//恢复CS端为高电平
	SPI_CS(1);
}

/**
 * @brief 写数据到W25Q128，不擦除扇区（要求目标区域已是0xFF）
 */
static void W25Q128_write_raw(uint8_t* buffer, uint32_t addr, uint16_t numbyte)
{
    unsigned int i = 0;
    W25Q128_write_enable();
    W25Q128_wait_busy();
    SPI_CS(0);
    spi_read_write_byte(0x02);
    spi_read_write_byte((uint8_t)(addr >> 16));
    spi_read_write_byte((uint8_t)(addr >> 8));
    spi_read_write_byte((uint8_t)addr);
    for (i = 0; i < numbyte; i++) {
        spi_read_write_byte(buffer[i]);
    }
    SPI_CS(1);
    W25Q128_wait_busy();
}

/**
 * @brief 页写入（不擦除），最大 256 字节，供外部调用
 */
void W25Q128_write_page(uint8_t* buffer, uint32_t addr, uint16_t numbyte)
{
    W25Q128_write_raw(buffer, addr, numbyte);
}

/**
 * @brief 保存系统参数到Flash（wear leveling）
 * @param data 5字节参数数组
 * @note  使用最后一个扇区（0xFFF000）存储，每条记录6字节（1字节标记+5字节数据），
 *        写满682条后自动擦除重写，寿命约为直接写入的682倍
 */
void settings_save(uint8_t* data)
{
    uint8_t magic;
    uint32_t slot;
    uint32_t max_slots = 4096 / SETTINGS_RECORD_SIZE;

    // 找下一个空槽（0xFF表示未写入）
    for (slot = 0; slot < max_slots; slot++) {
        W25Q128_read(&magic, SETTINGS_SECTOR_ADDR + slot * SETTINGS_RECORD_SIZE, 1);
        if (magic == 0xFF) break;
    }

    // 扇区已满，擦除后从头写
    if (slot >= max_slots) {
        W25Q128_erase_sector(SETTINGS_SECTOR_ADDR / 4096);
        slot = 0;
    }

    // 写入：标记 + 数据
    uint8_t record[SETTINGS_RECORD_SIZE];
    uint8_t i;
    record[0] = SETTINGS_MAGIC;
    for (i = 0; i < SETTINGS_DATA_SIZE; i++) {
        record[i + 1] = data[i];
    }
    W25Q128_write_raw(record, SETTINGS_SECTOR_ADDR + slot * SETTINGS_RECORD_SIZE, SETTINGS_RECORD_SIZE);
}

/**
 * @brief 从Flash读取最新的系统参数（wear leveling）
 * @param data 5字节参数数组，若无有效记录则不修改
 */
void settings_load(uint8_t* data)
{
    uint8_t magic;
    int32_t last_slot = -1;
    uint32_t slot;
    uint32_t max_slots = 4096 / SETTINGS_RECORD_SIZE;

    // 扫描找最后一条有效记录
    for (slot = 0; slot < max_slots; slot++) {
        W25Q128_read(&magic, SETTINGS_SECTOR_ADDR + slot * SETTINGS_RECORD_SIZE, 1);
        if (magic == SETTINGS_MAGIC) {
            last_slot = (int32_t)slot;
        } else if (magic == 0xFF) {
            break; // 到达空白区域
        }
    }

    if (last_slot >= 0) {
        W25Q128_read(data, SETTINGS_SECTOR_ADDR + (uint32_t)last_slot * SETTINGS_RECORD_SIZE + 1, SETTINGS_DATA_SIZE);
    } else {
        // 无有效记录但扇区非空（旧格式残留），擦除确保后续写入干净
        W25Q128_read(&magic, SETTINGS_SECTOR_ADDR, 1);
        if (magic != 0xFF) {
            W25Q128_erase_sector(SETTINGS_SECTOR_ADDR / 4096);
        }
    }
}

void W25Q128_test(void)
{
    unsigned char rec_buff[50]={0};
    unsigned char uart_output_buff[50]={0};

    delay_ms(10);
	//读取W25Q128的ID
    sprintf((char*)uart_output_buff,"FLASH ID = %X\r\n",W25Q128_readID());
    debug_uart_send_string((char*)uart_output_buff);

	//读取0地址的5个字节数据到buff
    W25Q128_read((uint8_t*)rec_buff, 0, 5);
    //串口输出读取的数据
    sprintf((char*)uart_output_buff, "1 buff = %s\r\n",rec_buff);
    debug_uart_send_string((char*)uart_output_buff);

    //往0地址写入5个字节长度的数据 lckfb
    W25Q128_write("lckfb", 0, 5);

    //读取0地址的5个字节数据到buff
    W25Q128_read((uint8_t*)rec_buff, 0, 5);
    //串口输出读取的数据
    sprintf((char*)uart_output_buff, "2 buff = %s\r\n", rec_buff);
    debug_uart_send_string((char*)uart_output_buff);
}

/**
 * @brief 写入单个字节到 Flash（不擦除，需提前擦除扇区）
 * @param addr 字节地址
 * @param data 要写入的字节
 */
void W25Q128_write_byte(uint32_t addr, uint8_t data)
{
    W25Q128_write_enable();
    W25Q128_wait_busy();
    SPI_CS(0);
    spi_read_write_byte(0x02);
    spi_read_write_byte((uint8_t)(addr >> 16));
    spi_read_write_byte((uint8_t)(addr >> 8));
    spi_read_write_byte((uint8_t)addr);
    spi_read_write_byte(data);
    SPI_CS(1);
    W25Q128_wait_busy();
}

/**
 * @brief 擦除字库存储区域（0x000000 ~ 0x03FFFF，共 64 个扇区）
 * @note  擦除后所有字节为 0xFF，耗时约数秒
 */
void W25Q128_erase_font_area(void)
{
    uint32_t sector;
    uint32_t total_sectors = FONT_AREA_END_ADDR / 4096;  // 64 个扇区
    for (sector = 0; sector < total_sectors; sector++) {
        W25Q128_erase_sector(sector);
    }
}