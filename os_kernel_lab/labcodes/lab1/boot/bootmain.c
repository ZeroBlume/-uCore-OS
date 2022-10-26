#include <defs.h>
#include <x86.h>
#include <elf.h>

/* *********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program(bootasm.S and bootmain.c) is the bootloader.
 *    It should be stored in the first sector of the disk.
 *
 *  * The 2nd sector onward holds the kernel image.
 *
 *  * The kernel image must be in ELF format.
 *
 * BOOT UP STEPS
 *  * when the CPU boots it loads the BIOS into memory and executes it
 *
 *  * the BIOS intializes devices, sets of the interrupt routines, and
 *    reads the first sector of the boot device(e.g., hard-drive)
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over...
 *
 *  * control starts in bootasm.S -- which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain()
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 * */

#define SECTSIZE 512                      // 扇区大小
#define ELFHDR ((struct elfhdr *)0x10000) // scratch space，将内核加载到内存RAM的起始地址

/* waitdisk - wait for disk ready */
static void
waitdisk(void)
{
    // 0x1F7: 状态和命令寄存器。操作时先给命令，再读取，如果不是忙状态就从0x1f0端口读数据
    // 最高位为0，次高位为1时表示不忙
    // 0xC0: 1100 0000 0x40: 0100 0000
    while ((inb(0x1F7) & 0xC0) != 0x40)
        /* do nothing */;
}

/* readsect - read a single sector at @secno into @dst */
// 从设备的第secno扇区读取数据到dst位置
static void
readsect(void *dst, uint32_t secno)
{
    // wait for disk to be ready
    waitdisk();

    // 0x1f2: 要读写的扇区数，每次读写前，你需要表明你要读写几个扇区。最小是1个扇区
    outb(0x1F2, 1); // count = 1
    // 读取的扇区起始编号共28位，分成4部分依次放在0x1F3~0x1F6寄存器中
    outb(0x1F3, secno & 0xFF);
    outb(0x1F4, (secno >> 8) & 0xFF);
    outb(0x1F5, (secno >> 16) & 0xFF);
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);
    // 发出读取扇区的命令。对应的命令字为0x20，放在0x1F7寄存器中
    outb(0x1F7, 0x20); // cmd 0x20 - read sectors

    // wait for disk to be ready
    waitdisk();

    // read a sector
    // 开始从0x1F0寄存器中读数据
    // insl: 是以dword即4字节为单位的，因此这里SECTIZE需要除以4.
    insl(0x1F0, dst, SECTSIZE / 4);
}

/* *
 * readseg - read @count bytes at @offset from kernel into virtual address @va,
 * might copy more than asked.
 * */
// 从ELF文件偏移为offset处读取count个字节va处
static void
readseg(uintptr_t va, uint32_t count, uint32_t offset)
{
    // 结束地址
    uintptr_t end_va = va + count;

    // round down to sector boundary
    // 将va设置为512字节对齐的地方
    va -= offset % SECTSIZE;

    // translate from bytes to sectors; kernel starts at sector 1
    // 开始扇区号
    uint32_t secno = (offset / SECTSIZE) + 1;

    // If this is too slow, we could read lots of sectors at a time.
    // We'd write more to memory than asked, but it doesn't matter --
    // we load in increasing order.
    for (; va < end_va; va += SECTSIZE, secno++)
    {
        readsect((void *)va, secno);
    }
}

/* bootmain - the entry of bootloader */
void bootmain(void)
{
    // read the 1st page off disk
    // 首先读取ELF的头部
    // 第一个参数是一个虚拟地址va(virtual address)
    // 第二个是count（我们所要读取的数据的大小 512*8），
    // 第三个是offset（偏移量）
    // 将硬盘从第一个扇区开始的4096个字节读到内存地址为0x10000处
    readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);

    // is this a valid ELF?
    if (ELFHDR->e_magic != ELF_MAGIC)
    {
        goto bad;
    }

    struct proghdr *ph, *eph;

    // load each program segment (ignores ph flags)
    // ELF头部有描述ELF文件应加载到内存什么位置的描述表，
    // 先将描述表的头地址存在ph
    ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
    eph = ph + ELFHDR->e_phnum;
    // 按照描述表将ELF文件中数据载入内存
    for (; ph < eph; ph++)
    {
        readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
    }
    // ELF文件0x1000位置后面的0xd1ec比特被载入内存0x00100000
    // ELF文件0xf000位置后面的0x1d20比特被载入内存0x0010e000

    // call the entry point from the ELF header
    // note: does not return
    // 根据ELF头部储存的入口信息，找到内核的入口
    ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();

bad:
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x8E00);

    /* do nothing */
    while (1)
        ;
}
