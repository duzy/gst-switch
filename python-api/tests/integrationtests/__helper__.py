l = open("logs").readlines()
for i in l:
	if i.find('gst-switch') < 0:
		if len(i) >2:
			print i