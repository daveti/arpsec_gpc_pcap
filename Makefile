# Makefile for arpsec_gpc_pcap
# Jan 18, 2014
# daveti@cs.uoregon.edu
# http://davejingtian.org

CC2=gplc
LIBS2='-lgcrypt -lpthread -ltspi -lpcap -lnet -lnetlink'
CC=gcc
LIBS=-lgcrypt -ltspi -lpthread

all: arpsecd_gpc_pcap tpmd checkproc

arpsecd_gpc_pcap: arpsec.pl arpsecd.c AsLog.c AsLogic.c AsTMeasure.c AsKrnRelay.c AsNetlink.c AsTpmDB.c AsWhiteList.c AT.c tpmw.c timer_queue.c timer_thread.c AsPcap.c AsKrnProc.c AsNet.c AsNeighbor.c AsControl.c
	$(CC2) arpsec.pl arpsecd.c AsLog.c AsLogic.c AsTMeasure.c AsKrnRelay.c AsNetlink.c AsTpmDB.c AsWhiteList.c AT.c tpmw.c timer_queue.c timer_thread.c AsPcap.c AsKrnProc.c AsNet.c AsNeighbor.c AsControl.c --c-compiler $(CC) -L $(LIBS2) -o arpsecd_gpc_pcap

tpmd: tpmd.c tpmw.c AT.c
	$(CC) tpmd.c tpmw.c AT.c $(LIBS) -o tpmd

checkproc: checkproc.c
	$(CC) checkproc.c -o checkproc

clean:
	rm -rf arpsecd_gpc_pcap tpmd checkproc
