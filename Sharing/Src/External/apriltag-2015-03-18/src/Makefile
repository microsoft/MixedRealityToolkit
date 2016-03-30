CC = gcc
AR = ar

CFLAGS = -std=gnu99 -Wall -Wno-unused-parameter -Wno-unused-function -pthread -I. -Icommon -O3
LDFLAGS = -lpthread -lm

APRILTAG_OBJS = apriltag.o apriltag_quad_thresh.o tag16h5.o tag25h7.o tag25h9.o tag36h10.o tag36h11.o tag36artoolkit.o g2d.o common/zarray.o common/zhash.o common/zmaxheap.o common/unionfind.o common/matd.o common/image_u8.o common/pnm.o common/image_f32.o common/image_u32.o common/workerpool.o common/time_util.o common/svd22.o common/homography.o common/string_util.o common/getopt.o

LIBAPRILTAG := libapriltag.a

all: $(LIBAPRILTAG)


$(LIBAPRILTAG): $(APRILTAG_OBJS)
	@echo "   [$@]"
	@$(AR) -cq $@ $(APRILTAG_OBJS)

apriltag_demo: apriltag_demo.o
	@echo "   [$@]"
	@$(CC) -o $@ apriltag_demo.o $(APRILTAG_OBJS) $(LDFLAGS)

%.o: %.c
	@echo "   $@"
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean
clean:
	@rm -rf *.o common/*.o $(LIBAPRILTAG) apriltag_demo
