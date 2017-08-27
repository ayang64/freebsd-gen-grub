#vim: ts=2:sw=2
target	:=	freebsd-gen-grub
src			:=	$(target).c
obj			:=	$(src:.c=.o)
cc			:=	$(CC)
#CFLAGS	:=	-MMD -std=gnu99 -march=native -Wall -W
CFLAGS	:=	-ggdb -MMD -std=gnu99 -march=native -Wall -W
libs		:=	-lparted
rm			:=	/bin/rm

.PHONY : clean

$(target)	: $(obj)
	$(cc) $(obj) -o $(target) $(libs)

-include *.d

clean:
	$(rm)	-f $(target) $(obj) *.d
