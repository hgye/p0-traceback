2015-8-1
需要解决的问题：
1. 知道一个函数的地址之后，怎么快速的在functions数组中查找到对应的元素？
答：这个数组是按函数地址排序的，那我用一个binary search就行了

2. 回溯call stack的时候，怎么知道是时候停止了？
答：按理说一个program的入口是main()，所以回溯到main()就可以了。

要获取stack frame的信息，最有效的方法就是用汇编了，所以traceback()一定需要
调用汇编实现的函数


Milestones:
阅读所有test文件，搞懂程序的正确输出应该是怎样的

1. 用汇编实现一个小函数：打印出当前stack frame的函数名、所有参数
2. 改进这个小函数能打印出完整的function call chain
3. 打印参数的值，覆盖handout上的所有要求


2015-8-20
1. 如果能拿到caller的起始地址，那么我就能在functions数组里查找到对应的entry，
但是怎么获取到caller function的起始地址？
答：函数的起始地址就是它第一条指令的地址，可是通过汇编代码回溯call stack的时候，
能拿到的地址就只有caller的返回地址，而返回地址距离函数的起始地址是有一个偏移量的，
而这个偏移量是个变量，没有渠道能获取到这个偏移量，想根据返回地址推算出函数的起始地址，这条路是行不通的。
我花了3~4天的时间来思考“如何获取到函数的起始地址”这个问题，但是始终没有任何进展。
我为了确认functions里存的到底是不是函数的起始地址，我把functions数组里的addr输出了，
证实里面存的的确是函数的起始地址，于是思考陷入了僵局。

2015-8-21
今天上午，我在osdev.org/Stack_Trace里看到了关键的一句话"When looking up the name of a function 
you have to find **the biggest address smaller than the value you are looking for**"。

第一眼看的时候，感觉这应该是我想要的答案，后来验证了一下，发现这的确是一个可行的办法！
思路是这样的：
你不是已经拿到了caller的返回地址吗？那么其实你要解决的问题是"这个返回地址是哪一个函数的返回地址？"，
之前的问题“如何获取到caller的起始地址？”是一个错误的思考方向，正因为你提了一个错误的问题，然后思维
就被这个问题牢牢框住了，走到死胡同里了。

回过来看，如何确定一个返回地址是属于哪一个函数的呢？首先，任意一个返回地址都是落在函数的地址区间里的，
即[begin_addr, begin_addr + sizeof(function) )这样的区间。并且functions数组是按照函数起始地址排好序的，
那么我就顺序遍历这个数组，拿返回地址 p 跟每一个函数的起始地址作比较，找出所有起始地址小于返回地址p的函数，
这些函数构成集合A。

然后就要在A中找到那唯一一个函数，判断条件就是A中起始地址最大的函数，p就是落在它的地址区间里！这就是我们
要找的caller。

2015-8-22

伪代码：
/**
 * 回溯到main函数就停止回溯
 *
 */
traceback():
do:
    funcsym_t func = get_next_func()
    if (func == NULL)
        break

    print_func(func)

while func.name != "main"


/**
 * get_next_func()参考了python的generator，是个不可
 * 重入的函数，它是有状态的; the variable ebp
 * traceback()不会在多线程场景下被调用，所以可以
 * 实现成 线程不安全的。
 *
 * @return pointer to a entry in functions array
 */
get_next_func():
    static var ebp = %ebp of current frame
    ret_addr = ebp + 4
    func = find_caller(ret_addr)
    // 这里必须要更新ebp，否则每次取的都只是第一个caller
    ebp = %ebp of next caller frame (if any)
    return func

需要解决的问题：
怎么在汇编代码里声明static local variable？
static scope的变量是放在哪个segment里的？
未初始化的，初始化为0的放在.bss
用非零值初始化的放在.data





