obj-y := core.o

obj-$(CONFIG_BPF_SYSCALL) += syscall.o verifier.o inode.o helpers.o
obj-$(CONFIG_BPF_SYSCALL) += hashtab.o arraymap.o


ifdef CONFIG_CC_CLOSE_OPTIMIZATION
CFLAGS_core.o	+= -O2
endif

