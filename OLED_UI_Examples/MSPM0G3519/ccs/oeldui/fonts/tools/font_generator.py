#!/usr/bin/env python3
"""
OLED_UI 字库生成工具 (带 GUI)

生成 GB2312 标准点阵字库 bin 文件，用于烧录到 W25Q128 外部 Flash。
字模格式：阴码、列行式、逆向（低位在前）
支持字号：12x12 (24字节/字)、16x16 (32字节/字)、20x20 (60字节/字)

GB2312 覆盖 87 个区 (0xA1-0xF7)，每区 94 位，共 8178 个汉字。
"""

import struct
import os
import sys
import threading
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("需要安装 Pillow 库: pip install Pillow")
    sys.exit(1)


# GB2312 参数
GB2312_AREA_START = 0xA1
GB2312_AREA_END = 0xF7  # 87个区
GB2312_POS_START = 0xA1
GB2312_POS_END = 0xFE    # 每区94位
GB2312_AREAS = GB2312_AREA_END - GB2312_AREA_START + 1  # 87
GB2312_POSITIONS = GB2312_POS_END - GB2312_POS_START + 1  # 94
GB2312_TOTAL_CHARS = GB2312_AREAS * GB2312_POSITIONS  # 8178

# 字号配置: (宽, 高, 每字节数, 页数)
FONT_CONFIG = {
    12: {"width": 12, "height": 12, "char_size": 24, "pages": 2},
    16: {"width": 16, "height": 16, "char_size": 32, "pages": 2},
    20: {"width": 20, "height": 20, "char_size": 60, "pages": 3},
}


def gb2312_to_unicode(high, low):
    """将 GB2312 编码转换为 Unicode 字符"""
    try:
        return bytes([high, low]).decode("gb2312")
    except (UnicodeDecodeError, ValueError):
        return None


def render_glyph(char, font_obj, width, height):
    """渲染单个字符为像素矩阵，返回 width x height 的二维列表"""
    img = Image.new("1", (width * 2, height * 2), 0)
    draw = ImageDraw.Draw(img)
    draw.text((0, 0), char, font=font_obj, fill=1)

    bbox = img.getbbox()
    if bbox is None:
        return [[0] * height for _ in range(width)]

    char_img = img.crop(bbox)
    cw, ch = char_img.size

    # 居中放置到目标尺寸
    final = Image.new("1", (width, height), 0)
    ox = max(0, (width - cw) // 2)
    oy = max(0, (height - ch) // 2)
    paste_w = min(cw, width - ox)
    paste_h = min(ch, height - oy)
    region = char_img.crop((0, 0, paste_w, paste_h))
    final.paste(region, (ox, oy))

    # 转为列优先像素矩阵 pixels[col][row]
    pixels = []
    for x in range(width):
        col = []
        for y in range(height):
            col.append(final.getpixel((x, y)))
        pixels.append(col)
    return pixels


def pixels_to_bytes(pixels, width, height, pages):
    """
    将像素矩阵转换为字模字节数组
    格式：列行式、逆向（低位在前）、阴码

    数据按页组织，每页8行：
      Page 0: col0[row0-7], col1[row0-7], ..., colN[row0-7]
      Page 1: col0[row8-15], col1[row8-15], ..., colN[row8-15]
      ...
    每字节中 bit0=该页最上面的行, bit7=该页最下面的行
    """
    data = bytearray()
    for page in range(pages):
        row_start = page * 8
        for col in range(width):
            byte_val = 0
            for bit in range(8):
                row = row_start + bit
                if row < height and pixels[col][row]:
                    byte_val |= (1 << bit)
            data.append(byte_val)
    return data


def generate_font_bin(font_path, font_size_key, output_path, progress_cb=None, log_cb=None):
    """
    生成字库 bin 文件

    font_path: 系统字体文件路径 (.ttf/.ttc/.otf)
    font_size_key: 12, 16, 或 20
    output_path: 输出 bin 文件路径
    progress_cb: 进度回调 (current, total)
    log_cb: 日志回调 (message)
    """
    cfg = FONT_CONFIG[font_size_key]
    width = cfg["width"]
    height = cfg["height"]
    char_size = cfg["char_size"]
    pages = cfg["pages"]

    # 渲染用的字体大小需要略小于像素尺寸以留出边距
    render_size = font_size_key
    font_obj = ImageFont.truetype(font_path, render_size)

    if log_cb:
        log_cb(f"字体: {os.path.basename(font_path)}")
        log_cb(f"字号: {width}x{height}, 每字 {char_size} 字节, {pages} 页")
        log_cb(f"总字符数: {GB2312_TOTAL_CHARS}")
        log_cb("开始生成...")

    bin_data = bytearray()
    generated = 0
    failed = 0

    for area in range(GB2312_AREA_START, GB2312_AREA_END + 1):
        for pos in range(GB2312_POS_START, GB2312_POS_END + 1):
            char = gb2312_to_unicode(area, pos)
            if char:
                pixels = render_glyph(char, font_obj, width, height)
                glyph_bytes = pixels_to_bytes(pixels, width, height, pages)
            else:
                glyph_bytes = bytearray(char_size)
                failed += 1

            bin_data.extend(glyph_bytes)
            generated += 1

            if progress_cb and generated % 100 == 0:
                progress_cb(generated, GB2312_TOTAL_CHARS)

    # 末尾 2 字节 padding（兼容 PCtoLCD2002 格式）
    bin_data.extend(b'\x00\x00')

    with open(output_path, "wb") as f:
        f.write(bin_data)

    if progress_cb:
        progress_cb(GB2312_TOTAL_CHARS, GB2312_TOTAL_CHARS)

    if log_cb:
        log_cb(f"完成! 成功: {generated - failed}, 失败: {failed}")
        log_cb(f"文件大小: {len(bin_data)} 字节")
        log_cb(f"输出: {output_path}")

    return len(bin_data)


def get_system_fonts():
    """获取 Windows 系统字体列表"""
    fonts = {}
    font_dir = os.path.join(os.environ.get("WINDIR", r"C:\Windows"), "Fonts")
    if os.path.isdir(font_dir):
        for f in os.listdir(font_dir):
            if f.lower().endswith((".ttf", ".ttc", ".otf")):
                full = os.path.join(font_dir, f)
                try:
                    fobj = ImageFont.truetype(full, 16)
                    name = fobj.getname()
                    display_name = f"{name[0]} ({name[1]})" if name[1] else name[0]
                    fonts[display_name] = full
                except Exception:
                    fonts[f] = full
    return fonts


def preview_chars(font_path, font_size_key, chars="你好世界测试"):
    """预览指定字符的字模渲染效果，返回 PIL Image"""
    cfg = FONT_CONFIG[font_size_key]
    width = cfg["width"]
    height = cfg["height"]
    pages = cfg["pages"]

    font_obj = ImageFont.truetype(font_path, font_size_key)

    scale = max(1, 128 // height)
    n = len(chars)
    img_w = n * width * scale + (n - 1) * scale
    img_h = height * scale
    img = Image.new("1", (img_w, img_h), 0)

    for idx, ch in enumerate(chars):
        gb_bytes = None
        try:
            gb_bytes = ch.encode("gb2312")
        except UnicodeEncodeError:
            pass

        if gb_bytes and len(gb_bytes) == 2:
            pixels = render_glyph(ch, font_obj, width, height)
            glyph_bytes = pixels_to_bytes(pixels, width, height, pages)

            # 从字节还原像素（验证编码正确性）
            for page in range(pages):
                for col in range(width):
                    byte_val = glyph_bytes[page * width + col]
                    for bit in range(8):
                        row = page * 8 + bit
                        if row < height and (byte_val & (1 << bit)):
                            px = idx * (width * scale + scale) + col * scale
                            py = row * scale
                            for dx in range(scale):
                                for dy in range(scale):
                                    if px + dx < img_w and py + dy < img_h:
                                        img.putpixel((px + dx, py + dy), 1)

    return img


class FontGeneratorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("OLED_UI 字库生成工具")
        self.root.resizable(True, True)
        self.root.minsize(700, 580)

        self.fonts = {}
        self.generating = False
        self.preview_photo = None

        self._build_ui()
        self._load_fonts()

    def _build_ui(self):
        main = ttk.Frame(self.root, padding=10)
        main.pack(fill=tk.BOTH, expand=True)

        # 字体选择
        row0 = ttk.LabelFrame(main, text="字体设置", padding=8)
        row0.pack(fill=tk.X, pady=(0, 8))

        ttk.Label(row0, text="系统字体:").grid(row=0, column=0, sticky=tk.W, padx=(0, 5))
        self.font_var = tk.StringVar()
        self.font_combo = ttk.Combobox(row0, textvariable=self.font_var, state="readonly", width=40)
        self.font_combo.grid(row=0, column=1, sticky=tk.EW, padx=(0, 5))
        self.font_combo.bind("<<ComboboxSelected>>", lambda e: self._update_preview())
        self.font_combo.bind("<Up>", self._font_prev)
        self.font_combo.bind("<Down>", self._font_next)

        ttk.Label(row0, text="或自选:").grid(row=0, column=2, padx=(10, 5))
        self.custom_path_var = tk.StringVar()
        ttk.Entry(row0, textvariable=self.custom_path_var, width=30).grid(row=0, column=3, sticky=tk.EW)
        ttk.Button(row0, text="浏览", command=self._browse_font).grid(row=0, column=4, padx=(5, 0))

        row0.columnconfigure(1, weight=1)
        row0.columnconfigure(3, weight=1)

        # 字号选择
        row1 = ttk.LabelFrame(main, text="生成选项", padding=8)
        row1.pack(fill=tk.X, pady=(0, 8))

        ttk.Label(row1, text="字号:").grid(row=0, column=0, sticky=tk.W, padx=(0, 5))
        self.size_var = tk.IntVar(value=16)
        for i, sz in enumerate([12, 16, 20]):
            cfg = FONT_CONFIG[sz]
            text = f"{sz}x{sz} ({cfg['char_size']}字节/字)"
            ttk.Radiobutton(row1, text=text, variable=self.size_var, value=sz,
                            command=self._update_preview).grid(row=0, column=i + 1, padx=10)

        ttk.Label(row1, text="格式:").grid(row=1, column=0, sticky=tk.W, padx=(0, 5), pady=(8, 0))
        ttk.Label(row1, text="阴码 | 列行式 | 逆向(低位在前) | GB2312 87区×94位 = 8178字",
                  foreground="gray").grid(row=1, column=1, columnspan=4, sticky=tk.W, pady=(8, 0))

        # 输出路径
        row2 = ttk.LabelFrame(main, text="输出设置", padding=8)
        row2.pack(fill=tk.X, pady=(0, 8))

        ttk.Label(row2, text="输出目录:").grid(row=0, column=0, sticky=tk.W, padx=(0, 5))
        self.output_dir_var = tk.StringVar(
            value=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "output")
        )
        ttk.Entry(row2, textvariable=self.output_dir_var, width=50).grid(row=0, column=1, sticky=tk.EW)
        ttk.Button(row2, text="浏览", command=self._browse_output).grid(row=0, column=2, padx=(5, 0))
        row2.columnconfigure(1, weight=1)

        # 预览
        preview_frame = ttk.LabelFrame(main, text="字模预览 (从字节还原验证)", padding=8)
        preview_frame.pack(fill=tk.X, pady=(0, 8))

        prev_top = ttk.Frame(preview_frame)
        prev_top.pack(fill=tk.X)
        ttk.Label(prev_top, text="预览文字:").pack(side=tk.LEFT, padx=(0, 5))
        self.preview_text_var = tk.StringVar(value="你好世界")
        self.preview_text_var.trace_add("write", lambda *_: self._schedule_preview())
        ttk.Entry(prev_top, textvariable=self.preview_text_var, width=20).pack(side=tk.LEFT, padx=(0, 5))
        ttk.Button(prev_top, text="刷新预览", command=self._update_preview).pack(side=tk.LEFT)

        self.preview_label = ttk.Label(preview_frame, background="black", anchor=tk.CENTER)
        self.preview_label.pack(fill=tk.X, pady=(8, 0), ipady=10)

        # 生成按钮和进度
        btn_frame = ttk.Frame(main)
        btn_frame.pack(fill=tk.X, pady=(0, 8))

        self.gen_btn = ttk.Button(btn_frame, text="生成字库 BIN", command=self._start_generate)
        self.gen_btn.pack(side=tk.LEFT)

        self.gen_all_btn = ttk.Button(btn_frame, text="一键生成全部 (12+16+20)", command=self._start_generate_all)
        self.gen_all_btn.pack(side=tk.LEFT, padx=(10, 0))

        self.progress = ttk.Progressbar(btn_frame, mode="determinate")
        self.progress.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(10, 0))

        # 日志
        log_frame = ttk.LabelFrame(main, text="日志", padding=4)
        log_frame.pack(fill=tk.BOTH, expand=True)

        self.log_text = tk.Text(log_frame, height=8, state=tk.DISABLED, font=("Consolas", 9))
        scrollbar = ttk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.log_text.yview)
        self.log_text.configure(yscrollcommand=scrollbar.set)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.log_text.pack(fill=tk.BOTH, expand=True)

    def _load_fonts(self):
        self.log("正在扫描系统字体...")
        self.fonts = get_system_fonts()
        names = sorted(self.fonts.keys())
        self.font_combo["values"] = names

        # 优先选择中文字体
        preferred = ["SimSun", "宋体", "SimHei", "黑体", "Microsoft YaHei", "微软雅黑"]
        for pref in preferred:
            for name in names:
                if pref.lower() in name.lower():
                    self.font_var.set(name)
                    self.log(f"已找到 {len(names)} 个系统字体，默认选择: {name}")
                    self._update_preview()
                    return

        if names:
            self.font_var.set(names[0])
            self.log(f"已找到 {len(names)} 个系统字体")
            self._update_preview()

    def _get_font_path(self):
        custom = self.custom_path_var.get().strip()
        if custom and os.path.isfile(custom):
            return custom
        name = self.font_var.get()
        return self.fonts.get(name)

    def _browse_font(self):
        path = filedialog.askopenfilename(
            title="选择字体文件",
            filetypes=[("字体文件", "*.ttf *.ttc *.otf"), ("所有文件", "*.*")]
        )
        if path:
            self.custom_path_var.set(path)
            self._update_preview()

    def _browse_output(self):
        d = filedialog.askdirectory(title="选择输出目录")
        if d:
            self.output_dir_var.set(d)

    def _font_prev(self, event):
        """键盘上键：切换到上一个字体"""
        values = self.font_combo["values"]
        if not values:
            return "break"
        cur = self.font_var.get()
        try:
            idx = list(values).index(cur)
        except ValueError:
            idx = 0
        idx = (idx - 1) % len(values)
        self.font_var.set(values[idx])
        self.font_combo.event_generate("<<ComboboxSelected>>")
        return "break"

    def _font_next(self, event):
        """键盘下键：切换到下一个字体"""
        values = self.font_combo["values"]
        if not values:
            return "break"
        cur = self.font_var.get()
        try:
            idx = list(values).index(cur)
        except ValueError:
            idx = -1
        idx = (idx + 1) % len(values)
        self.font_var.set(values[idx])
        self.font_combo.event_generate("<<ComboboxSelected>>")
        return "break"

    def _schedule_preview(self):
        """延迟刷新预览，避免每次按键都渲染"""
        if hasattr(self, "_preview_after_id"):
            self.root.after_cancel(self._preview_after_id)
        self._preview_after_id = self.root.after(200, self._update_preview)

    def _update_preview(self):
        font_path = self._get_font_path()
        if not font_path:
            return
        try:
            chars = self.preview_text_var.get().strip() or "你好世界"
            size_key = self.size_var.get()
            img = preview_chars(font_path, size_key, chars)

            # 转为 PhotoImage
            img_rgb = img.convert("RGB")
            from PIL import ImageTk
            self.preview_photo = ImageTk.PhotoImage(img_rgb)
            self.preview_label.configure(image=self.preview_photo)
        except Exception as e:
            self.log(f"预览失败: {e}")

    def log(self, msg):
        self.log_text.configure(state=tk.NORMAL)
        self.log_text.insert(tk.END, msg + "\n")
        self.log_text.see(tk.END)
        self.log_text.configure(state=tk.DISABLED)

    def _start_generate(self):
        if self.generating:
            return
        font_path = self._get_font_path()
        if not font_path:
            messagebox.showerror("错误", "请选择字体文件")
            return

        size_key = self.size_var.get()
        out_dir = self.output_dir_var.get().strip()
        if not os.path.isdir(out_dir):
            messagebox.showerror("错误", "输出目录不存在")
            return

        output_path = os.path.join(out_dir, f"HZK{size_key}.bin")
        self._run_generate(font_path, size_key, output_path)

    def _start_generate_all(self):
        if self.generating:
            return
        font_path = self._get_font_path()
        if not font_path:
            messagebox.showerror("错误", "请选择字体文件")
            return

        out_dir = self.output_dir_var.get().strip()
        if not os.path.isdir(out_dir):
            messagebox.showerror("错误", "输出目录不存在")
            return

        self._run_generate_all(font_path, out_dir)

    def _run_generate(self, font_path, size_key, output_path):
        self.generating = True
        self.gen_btn.configure(state=tk.DISABLED)
        self.gen_all_btn.configure(state=tk.DISABLED)
        self.progress["value"] = 0

        def progress_cb(current, total):
            self.root.after(0, lambda: self.progress.configure(value=current / total * 100))

        def log_cb(msg):
            self.root.after(0, lambda m=msg: self.log(m))

        def task():
            try:
                generate_font_bin(font_path, size_key, output_path, progress_cb, log_cb)
                # 同时生成索引 TXT
                txt_path = output_path.replace(".bin", ".TXT")
                self._generate_index_txt(txt_path, log_cb)
            except Exception as e:
                self.root.after(0, lambda: self.log(f"错误: {e}"))
            finally:
                self.root.after(0, self._on_generate_done)

        threading.Thread(target=task, daemon=True).start()

    def _run_generate_all(self, font_path, out_dir):
        self.generating = True
        self.gen_btn.configure(state=tk.DISABLED)
        self.gen_all_btn.configure(state=tk.DISABLED)
        self.progress["value"] = 0

        sizes = [12, 16, 20]
        total_chars = GB2312_TOTAL_CHARS * len(sizes)
        generated_total = [0]

        def progress_cb(current, total):
            val = (generated_total[0] + current) / total_chars * 100
            self.root.after(0, lambda: self.progress.configure(value=val))

        def log_cb(msg):
            self.root.after(0, lambda m=msg: self.log(m))

        def task():
            try:
                for sz in sizes:
                    output_path = os.path.join(out_dir, f"HZK{sz}.bin")
                    log_cb(f"\n{'='*40}")
                    log_cb(f"生成 HZK{sz}...")
                    generate_font_bin(font_path, sz, output_path, progress_cb, log_cb)
                    generated_total[0] += GB2312_TOTAL_CHARS

                    txt_path = output_path.replace(".bin", ".TXT")
                    self._generate_index_txt(txt_path, log_cb)

                log_cb(f"\n全部完成!")
            except Exception as e:
                self.root.after(0, lambda: self.log(f"错误: {e}"))
            finally:
                self.root.after(0, self._on_generate_done)

        threading.Thread(target=task, daemon=True).start()

    def _generate_index_txt(self, txt_path, log_cb):
        """生成 GB2312 索引文件（与 PCtoLCD2002 兼容）"""
        data = bytearray()
        for area in range(GB2312_AREA_START, GB2312_AREA_END + 1):
            for pos in range(GB2312_POS_START, GB2312_POS_END + 1):
                data.append(area)
                data.append(pos)
        # 末尾多一个条目（兼容 PCtoLCD2002）
        data.append(0x00)
        data.append(0x00)
        with open(txt_path, "wb") as f:
            f.write(data)
        if log_cb:
            log_cb(f"索引文件: {txt_path} ({len(data)} 字节)")

    def _on_generate_done(self):
        self.generating = False
        self.gen_btn.configure(state=tk.NORMAL)
        self.gen_all_btn.configure(state=tk.NORMAL)
        self.progress["value"] = 100


def main():
    root = tk.Tk()
    app = FontGeneratorApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
