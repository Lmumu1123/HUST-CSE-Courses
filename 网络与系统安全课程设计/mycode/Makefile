APP=pcap

all: 
	gcc -o pcap pcap-sample.c TCPFlowReconstruction.c -lpcap -lcrypto

clean: 
	rm -f $(APP)
	rm -f *~
