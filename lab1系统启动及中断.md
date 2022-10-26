![Snipaste_2022-10-09_22-46-20](E:\labs\Operating System\lab1系统启动及中断.assets\Snipaste_2022-10-09_22-46-20.png)

##### x86启动顺序

> 理解x86-32平台的启动过程
>
> 理解x86-32的实模式、保护模式
>
> 理解段机制

1. 寄存器初值: CS:EIP

2. 第一条指令：长跳转指令

3. 处于实模式的段：Base + Offset = Linear Address

4. 从BIOS到BootLoader

5. 从BootLoader到OS

   1. 使能保护模式(protection mode)&段机制segment-level protection

   2. 从硬盘上读取kernel in ELF格式的ucore kernel(跟在MBR后面的扇区)并放到内存中固定位置

      > elfhdr $\rightarrow$ proghdr

   3. 跳转到ucore OS的入口点(entry point)执行这时控制权到了ucore OS中

##### x86-32下的中断处理

> 了解x86中的中断源
> 了解CPU与操作系统如何处理中断
> 能够对中断向量表（中断描述符表，简称DT)进行初始化

1. 中断源
   1. 中断
      1. 外部中断
      2. 软中断：常用于系统调用
   2. 异常
      1. 程序错误
      2. 软件产生的异常
      3. 机器检查出来的异常
2. 中断处理
   1. 确定中断服务例程（ISR）
      1. 每个中断或异常与一个中断服务例程关联，其关联关系存储在中断描述符表中
      2. DT的起始地址和大小保存在中断描述符表寄存器DTR中
   2. 切换到中断服务例程
      1. 不同特权级的中断切换对堆栈的影响
   3. 从中断服务例程返回
      1. iret、ret、retf
   4. 系统调用

##### 小结



##### 练习1（理解执行make命令后生成的信息/理解makefile文件中命令的含义 + 理解sign.c的作用）

1. 操作系统镜像文件ucore.img是如何一步一步生成的？

   > 回答：
   >
   > 可以通过查看执行make命令后生成的信息或者查看makefile文件中的命令来观察ucore.img是如何一步一步生成的。
   >
   > ucore.img $\rightarrow$ bootblock + kernel:
   >
   > - gcc init.c、stdio.c、readline.c、panic.c、kdebug.c、kmonitor.c、clock.c、console.c、picirq.c、intr.c、trap.c、vectors.S、trapentry.S、pmm.c、string.c、printfmt.c  $\rightarrow$ … .o文件、已存在的kernel.ld $\rightarrow$ ld bin/**kernel**
   >
   >
   > - gcc bootasm.c、bootmain.c、sign.c $\rightarrow$ … .o文件 $\rightarrow$ ld bin/**bootlock**
   >
   >   细化：bootmain.c、bootsam.S $\stackrel{\mathrm{gcc -c}}{\rightarrow}$ bootmain.o、bootasm.o $\stackrel{\mathrm{ld}}{\rightarrow}$ bootlock.o
   >
   >   ​     sign.c $\rightarrow$ sign.o $\rightarrow$ sign
   >
   >   ​     拷贝二进制代码bootblock.o到bootblock.out（移除所有符号和重定位信息），其中bootblock.out' size:     
   >
   >   ​     500 bytes (主引导扇区只有512字节，bootloader的最终大小不能大于510字节)使用sign工具处理  
   >
   >   ​     bootblock.out，生成bootblock
   >
   > - 生成一个有10000个块的文件，每个块默认512字节，用0填充
   >
   >   dd if=/dev/zero of=bin/ucore.img count=10000
   >
   >   把bootblock中的内容写到第一个块
   >
   >   dd if=bin/bootblock of=bin/ucore.img conv=notrunc
   >
   >   从第二个块开始写kernel中的内容
   >
   >   dd if=bin/kernel of=bin/ucore.img seek=1 conv=notrunc

2. 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？

   > sign.c：打开bootlock.out $\rightarrow$ 读入buf[512]、并添加主引导扇区特征 $\rightarrow$ buf写入bootlock 
   >
   > 识别主引导扇区：buf[510] = 0x55; buf[511] = 0xAA;

3. *文件结构*

   > - ucore.img: 被qemu访问的虚拟硬盘文件
   > - kernel: ELF格式的toy ucore kernel执行文文件，被嵌入到了ucore.img中
   > - bootblock: 虚拟的硬盘主引导扇区（512字节），包含了bootloader执行代码，被嵌入到了ucore.img中
   > - sign：外部执行程序，用来生成虚拟的硬盘主引导扇区

##### 练习2（学习使用qemu执行并使用gdb进行调试）

1. 调试过程中会使用到的相关命令

   > - 源码级别调试：s(step)、n(next)、c
   > - 汇编级别调试：si(stepi)、ni(nexti)、c
   > - 退出正在调试的函数：finish(函数执行完并打印返回值)、return(函数不继续执行，直接返回，可指定返回值)
   > - 设置断点：
   >   - 地址：b *address, e.g. b *0x7d10
   >   - 行号：b linenum, e.g. b 23 或 b src/main.cpp:127
   >   - 变量：b &变量名
   >   - 函数：b funcName
   > - 显示源代码：
   >   - list：显示邻近的几行源代码
   >   - layout src/ctrl+X+A：显示源代码窗口
   >   - layout split：源代码和汇编代码都显示
   > - 显示汇编代码：
   >   - x /ni $pc：n表示要显示的汇编代码行数
   >   - layout asm：显示汇编代码窗口
   >   - layout split：源代码和汇编代码都显示
   >   - set disassemble-next-line on：自动反汇编后面要执行的代码
   > - 显示寄存器：layout regs
   > - 打印寄存器/变量的值：
   >   - i r 寄存器名称：e.g. i r eax
   >   - p $eax 、p \$变量
   > - 显示指定地址的内容：x/  <n/f/u>  \<addr\>

2. 使用qemu和gdb进行的调试工作的过程：

   > 窗口1：
   >
   > qemu -S -s -hda ucore.img -monitor stdio 
   >
   > - -S：在启动时不启动 CPU， 需要在 monitor 中输入 'c'，才能让qemu继续模拟工作。
   >
   > - -s：等待 gdb 连接到端口 1234。
   >
   > 窗口2：
   >
   > gdb ./bin/kernel   # 指定调试目标文件，让gdb获知符号信息
   >
   > target remote:1234 # 与qemu进行连接
   >
   > 然后就可以进行调试工作了

   > 调试练习：跟踪从kern_init到print_stackframe函数调用过程中ebp的值的变化。
   >
   > make debug后进入调试界面，layout split展示源代码和汇编代码，b 106在106行处设置断点后运行至此处，该段代码会根据ELF header找到内核的入口。si进入内核。此时可以观察到第一个调用信息。
   >
   > <img src="E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016011106104.png" alt="image-20221016011106104" style="zoom:150%;" />
   >
   > file ./bin/kernel指定调试目标文件为kernel，这样就可以显示相关源代码。设置断点到grade_backtrace()函数处，运行到此处后单步进入。此时可以观察到第二个调用信息。
   >
   > <img src="E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016011729263.png" alt="image-20221016011729263" style="zoom:150%;" />
   >
   > 设置断点到grade_backtrace0()函数处，运行到此处后单步进入。此时可以观察到第三个调用信息。
   >
   > <img src="E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016011931159.png" alt="image-20221016011931159" style="zoom:150%;" />
   >
   > 设置断点到grade_backtrace1()函数处，运行到此处后单步进入。此时可以观察到第四个调用信息。
   >
   > <img src="E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016012043290.png" alt="image-20221016012043290" style="zoom:150%;" />
   >
   > 设置断点到grade_backtrace2()函数处，运行到此处后单步进入。此时可以观察到第五个调用信息。
   >
   > ![image-20221016013717994](E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016013717994.png)
   >
   > 设置断点到mon_backtrace()函数处，运行到此处后单步进入。此时可以观察到第六个调用信息。
   >
   > <img src="E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016012156643.png" alt="image-20221016012156643" style="zoom:150%;" />
   >
   > 设置断点到print_stackframe()函数处，运行到此处后单步进入。此时可以观察到第七个调用信息。
   >
   > <img src="E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016013923813.png" alt="image-20221016013923813" style="zoom:150%;" />
   >
   > 当%esp的值赋给%ebp后可以得到第八个调用信息。
   >
   > ![image-20221016014133930](E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016014133930.png)
   >
   > 可以将此结果和make qemu后输出的栈帧打印结果比较可以发现是一致的。由于切换了调试目标文件，故打印栈帧的结果中调用kernel_init()函数的bootmain()函数的信息就丢失了。

##### 练习3（理解bootasm.S的作用）

1. 分析bootloader进入保护模式的过程

   > 1. 首先清理环境
   >
   >    - 通过cli指令将中断标志置0屏蔽系统中断，避免中断带来的干扰
   >    - 通过cld指令将方向标志置0，设置内存地址是向高地址增加
   >    - 将DS、ES、SS寄存器初始化为0。
   >
   > 2. 开启A20：
   >
   >    - 为什么要使能A20：在早期的8086CPU中，内存总线是20位的，可寻址空间范围即(00000H~FFFFFH)的1MB内存空间。但8086的数据处理位宽位16位，无法直接寻址1MB内存空间，所以8086提供了段地址加偏移地址的地址转换机制。由高16位的段基址和低16位的段内偏移共同构成一个20位的内存地址，但当FFFF:FFFF = 0xFFFF <<< 4 + 0xFFFF = 0x10FFEF，通过0x10FFEF的二进制表示可以看出它有个第21位，但是实际上内存总线只有20位，所以出现了溢出而8086中对这种溢出是兼容的，这种溢出在8086上会体现为绕回0x00000低端。但是在80286中，Intel把地址线扩展成24根了，FFFF:FFFF真的就是0x10FFEF了。这就造成了向下不兼容。为了保持完全的向下兼容性，于是出现了A20 Gate。如果A20 Gate被打开，则当程序员给出100000H-10FFEFH之间的地址的时候，系统将真正访问这块内存区域；如果A20 Gate被关闭，则系统仍然使用8086/8088的方式。
   >
   >    - 打开方法：可以通过向键盘控制器8042发送一个命令来控制A20地址线的打开和关闭。
   >
   >      - 等待8042 Input buffer为空
   >
   >        inb $0x64, %al  # 通过读0x64得到状态寄存器的值，存放到al寄存器中
   >
   >        testb $0x2, %al # 通过状态寄存器的定义可知，右往左第二位为1时8042 Input buffer非空，所以和0x2
   >
   >        ​				  进行与操作判断输入缓冲是否为空
   >
   >        jnz seta20.1    # 非零即输入缓存非空，则跳转直至8042 Input buffer为空
   >
   >      - 发送Write 8042 Output Port （P2）命令到8042 Input buffer
   >
   >        movb $0xd1, %al           # 0xd1 -> port 0x64 
   >
   >        outb %al, $0x64           # 0xd1 means: write data to 8042's P2 port，
   >
   >      - 等待8042 Input buffer为空
   >
   >      - 将8042 Output Port（P2）得到字节的第2位置1，然后写入8042 Input buffer
   >
   >        movb $0xdf, %al           # 0xdf -> port 0x60
   >
   >        outb %al, $0x60           # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1
   >
   > 3. 初始化GDT表：使得虚拟地址和物理地址匹配可以相互转换，可通过两种方式进行初始化
   >
   >    - 汇编代码初始化（bootasm.S）：lgdt gdtdesc # 把通过gdt处理后的（asm.h头文件中处理函数）描述符表的起始位置和大小加载入全局描述符表寄存器GDTR中。
   >    - C语言初始化（mm/pmm.c）：结构体数组struct segdesc gdt[]定义了gdt结构，gdt_init()函数进行初始化
   >
   > 4. 进入保护模式：CR0中包含了6个预定义标志，最低位是保护允许位PE(Protedted Enable)，用于启动保护模式，如果PE位
   >
   >    ​            置1，则保护模式启动，如果PE=0，则在实模式下运行。
   >
   > 5. 通过长跳转更新cs的基地址：ljmp \$PROT_MODE_CSEG, \$protcseg
   >
   > 6. 设置段寄存器，并建立堆栈：%ebp = 0x0；%esp = 0x7c00
   >
   > 7. 转到保护模式完成，进入boot主方法：call bootmain
   
2. bootasm.S的作用

   > bootasm.S：计算机加电后，由BIOS将bootasm.S生成的可执行代码从硬盘的第一个扇区复制到内存中的物理地址0x7c00处,并开始执行。此时系统处于实模式。可用内存不多于1M。bootasm.S就是要启动保护模式，转入C函数即bootmain.c。



##### 练习4（分析bootloader加载ELF格式的OS的过程，理解bootmain.c的作用）

1. bootloader如何读取硬盘扇区的

   bootloader访问硬盘时都是LBA模式的PIO方式，所有的I/O操作都是通过CPU访问硬盘的I/O地址寄存器完成。操作系统位于第一个硬盘上，而访问第一个硬盘的扇区可以设置I/O端口0x1f0~0x1f7来改变地址寄存器实现。

   | I/O地址     | 功能                                                         |
   | ----------- | :----------------------------------------------------------- |
   | 0x1f0       | 读数据，当0x1f7不为忙状态时，可以读                          |
   | 0x1f2       | 要读写的扇区数量，每次读写前，都需要表明要读写几个扇区       |
   | 0x1f3~0x1f6 | 存储要读写的起始扇区的编号（共28位）                         |
   | 0x1f7       | 状态和命令寄存器。操作时先给命令，再读取，如果不是忙状态就从0x1f0端口读数据 |

   > 读取扇区的功能通过bootmain.c的readsect函数实现，读一个扇区的大致流程：
   >
   > - 等待磁盘准备好：waitdisk()函数实现，while ((inb(0x1F7) & 0xC0) != 0x40);不断查询读0x1F7寄存器的最高两位，直到最高位为0、次高位为1（这个状态应该意味着磁盘空闲）才返回。
   >
   > - 发出读取扇区的命令：
   >
   >   向0x1F2写入要读写的扇区数量。再依次向0x1F3\~0x1F6写入读取的扇区起始编号（共28位），分成4部分依次放在0x1F3~0x1F6寄存器中。最后再向0x1F7发出读取扇区的命令。对应的命令字为0x20。
   >
   > - 等待磁盘准备好
   >
   > - 把磁盘扇区数据读到指定内存：通过insl函数从0x1F0寄存器中以dword即4字节为单位读数据到dst即目的地址中。

2. bootloader是如何加载ELF格式的OS

   ![image-20221016120129825](E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016120129825.png)

   ELF文件格式是Linux系统下的一种常用目标文件(object file)格式。ELF header在文件开始处描述了整个文件的组织。ELF header包含整个执行文件的控制结构，其定义在elf.h/elfhdr中。ELF header里面含有e_phoff字段，用于记录program header表在文件中的偏移，由该字段可以找到程序头表的起始地址。程序头表是一个结构体数组，其元素数目记录在ELF header的e_phnum字段中。

   rogram header描述与程序执行直接相关的目标文件结构信息，用来在文件中定位各个段的映像，同时包含其他一些用来为程序创建进程映像所必需的信息。

   > 1. 读取ELF文件头数据，即从硬盘读了8个扇区数据到ELFHDR指针指向的内存区域。
   >
   > 2. 校验e_magic字段，判断是否是合法的ELF文件。
   > 3. 根据ELF header的e_phoff字段，获得Program header的起始地址ph；通过ph和表示程序头表中的元素数目e_phnum获得获取程序头表结束的位置eph。
   > 4. 遍历Program Header表中的每个元素，得到每个Segment在文件中的偏移、要加载到内存中的位置（虚拟地址）及Segment的长度等信息，并通过磁盘I/O进行加载。通过反汇编可知一共有3个segment。
   > 5. 加载完毕，通过ELF Header的e_entry得到内核的入口地址，并跳转到该地址开始执行内核代码，即/kern/init.c中的kern_init函数。

3. bootmain.c的作用

   > bootloader引导程序是位于设备的第一个扇区，即引导扇区的，而ucore的内核程序则是从第二个磁盘扇区开始往后存放的。bootmain.c的任务就是将kernel内核部分从磁盘中读出并载入内存，并将程序的控制流转移至指定的内核入口处，即/kern/init.c中的kern_init函数。



##### 练习5（实现函数调用堆栈跟踪函数，理解函数调用的过程）

函数调用关系：call的时候函数的返回地址会入栈

> kern_init ->
> grade_backtrace ->
>    grade_backtrace0(0, (int)kern_init, 0xffff0000) ->
>            grade_backtrace1(0, 0xffff0000) ->
>                grade_backtrace2(0, (int)&0, 0xffff0000, (int)&(0xffff0000)) ->
>                    mon_backtrace(0, NULL, NULL) ->
>                        print_stackframe ->

<img src="E:\labs\Operating System\lab1系统启动及中断.assets\image-20221015212812444.png" alt="image-20221015212812444" style="zoom: 100%;" />

对于`ebp:0x00007bf8 eip:0x00007d73 args:0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8`共有ebp，eip和args三类参数

![image-20221016132237210](E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016132237210.png)

- `ebp:0x00007bf8`：bootmain函数栈帧的底部

- `eip:0x00007d73`：kern_init函数的返回地址，即bootmain函数调用kern_init对应的指令的下一条指令的地址

- `args:0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8`：args存放的是0x7c00地址的高16字节的内容

  ![image-20221016152052737](E:\labs\Operating System\lab1系统启动及中断.assets\image-20221016152052737.png)

##### 练习6（完善中断初始化和处理）

1. 中断描述符表（也可简称为保护模式下的中断向量表）中一个表项占多少字节？其中哪几位代表中断处理代码的入口？

   ```c++
   //kern/mm/mmu.h
   /* Gate descriptors for interrupts and traps */
   struct gatedesc {
       unsigned gd_off_15_0 : 16;        // low 16 bits of offset in segment
       unsigned gd_ss : 16;            // segment selector
       unsigned gd_args : 5;            // # args, 0 for interrupt/trap gates
       unsigned gd_rsv1 : 3;            // reserved(should be zero I guess)
       unsigned gd_type : 4;            // type(STS_{TG,IG32,TG32})
       unsigned gd_s : 1;                // must be 0 (system)
       unsigned gd_dpl : 2;            // descriptor(meaning new) privilege level
       unsigned gd_p : 1;                // Present
       unsigned gd_off_31_16 : 16;        // high bits of offset in segment
   };
   
   ```

   > 从该结构体可以看出中断描述符表一个表项占用8字节。其中gd_ss是段选择子，gd_off_15_0和gd_off_31_16合并成gd_off，通过gd_ss和gd_off可以确定中断处理程序的入口。

2. 编程完善kern/trap/trap.c中对中断向量表进行初始化的函数idt_init

   > 在保护模式下，最多会存在256个Interrupt/Exception Vectors。在kern/trap/vector.S中设置了这256个中断处理例程，并定义了一个指向函数（ISR的入口）的指针数组__vector。只需要在idt中的每一项填入对应中断的ISR入口即可，可以使用mmu.h中的宏SETGATE来对idt中的每一项进行填充。最后，通过lidt函数执行汇编指令lidt，完成了对IDTR寄存器的赋值。

   ```c++
   void idt_init(void)
   {
       extern uintptr_t __vectors[]; // 变量声明，是告诉编译器应该到该文件外部去找这个文件的定义
       for (int i = 0; i < sizeof(idt) / sizeof(struct gatedesc); i++)
       { 
           SETGATE(idt[i], 0, GD_KTEXT, __vectors[T_SWITCH_TOK], DPL_USER);
       }
       lidt(&idt_pd);
   }
   ```

3. 编程完善trap.c中的中断处理函数trap

   ```c++
   // 也可以设置不同类型的时间中断 
   ticks ++;
   if(ticks % TICK_NUM == 0)
   {
       print_ticks();
   }
   ```



##### 遇到的问题

1. 执行qemu后提示Command 'qemu' not found

   > 解决方法：使用qemu-system-i386或qemu-system-x86_64指令替换qemu指令。也可在使用sudo ln -s /usr/bin/qemu-system-i386 /usr/bin/qemu建立一条软链接后，直接使用qemu。删除软链接的方法：sudo rm -rf 映射目录
   
2. 完成print_stackframe函数后，在lab1中make grade后结果不正确，但是替换lab1_result中的print_stackframe函数后make grade结果正确。

   > peint_stackframe函数编写没有问题。
