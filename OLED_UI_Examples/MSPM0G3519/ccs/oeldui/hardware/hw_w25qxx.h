#ifndef _BSP_W25QXX_H__
#define _BSP_W25QXX_H__

#include "ti_msp_dl_config.h"

//CS引脚的输出控制
//x=0时输出低电平
//x=1时输出高电平
#define SPI_CS(x)  ( (x) ? DL_GPIO_setPins(FLASH_PORT,FLASH_CS_PIN) : DL_GPIO_clearPins(FLASH_PORT,FLASH_CS_PIN) )

// 系统参数存储配置
#define SETTINGS_SECTOR_ADDR   0xFFF000   // 使用最后一个扇区，避免与字体等数据冲突
#define SETTINGS_DATA_SIZE     13         // 参数数据字节数
#define SETTINGS_RECORD_SIZE   14         // 每条记录大小（1字节标记 + 13字节数据）
#define SETTINGS_MAGIC         0xAB       // 有效记录标记（修改后旧记录自动失效）

// 外部字库存储配置
// Flash 地址分配:
// 0x000000 ~ 0x03FFFF  HZK16      256KB
// 0x040000 ~ 0x04A3FF  U2G映射表    42KB
// 0x050000 ~ 0x07FFFF  HZK12      192KB
// 0x080000 ~ 0x0F7FFF  HZK20      480KB
// 0xFFF000 ~ 0xFFFFFF  系统参数      4KB

#define FONT16_BASE_ADDR       0x000000   // 16x16 字库起始地址
#define FONT16_CHAR_SIZE       32         // 每个 16x16 汉字占 32 字节
#define FONT16_TOTAL_SIZE      261698     // HZK16 文件大小
#define FONT16_END_ADDR        0x040000   // 16x16 字库区域结束地址

#define FONT12_BASE_ADDR       0x050000   // 12x12 字库起始地址
#define FONT12_CHAR_SIZE       24         // 每个 12x12 汉字占 24 字节
#define FONT12_TOTAL_SIZE      196274     // HZK12 文件大小
#define FONT12_END_ADDR        0x080000   // 12x12 字库区域结束地址

#define FONT20_BASE_ADDR       0x080000   // 20x20 字库起始地址
#define FONT20_CHAR_SIZE       60         // 每个 20x20 汉字占 60 字节
#define FONT20_TOTAL_SIZE      490682     // HZK20 文件大小
#define FONT20_END_ADDR        0x100000   // 20x20 字库区域结束地址

// Unicode → GB2312 映射表配置
#define U2G_MAP_BASE_ADDR      0x040000   // 映射表起始地址
#define U2G_MAP_UNICODE_START  0x4E00     // 映射表覆盖的 Unicode 起始码位
#define U2G_MAP_UNICODE_END    0x9FFF     // 映射表覆盖的 Unicode 结束码位
#define U2G_MAP_SIZE           41984      // 映射表大小 (20992 * 2 字节)
#define U2G_MAP_END_ADDR       0x04A400   // 映射表结束地址（不含）

// 兼容旧宏名
#define FONT_AREA_END_ADDR     FONT16_END_ADDR

uint16_t W25Q128_readID(void);
void W25Q128_write(uint8_t* buffer, uint32_t addr, uint16_t numbyte);
void W25Q128_read(uint8_t* buffer, uint32_t read_addr, uint16_t read_length);
void W25Q128_test(void);

// wear leveling 接口
void settings_save(uint8_t* data);
void settings_load(uint8_t* data);

// 字库烧录相关
void W25Q128_write_byte(uint32_t addr, uint8_t data);
void W25Q128_write_page(uint8_t* buffer, uint32_t addr, uint16_t numbyte);
void W25Q128_erase_font_area(void);

#endif