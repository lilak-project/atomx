make_new_file = True

input_name = "/home/ejungwoo/lilak/atomx/common/atomx_micromegas_mapping_v3.txt"
output_name = input_name.replace(".txt","_chan.txt")

print(input_name)
print(output_name)
exit()

file_in = open(input_name)
if make_new_file:
    file_out = open(output_name,'w')
    print(output_name)

lines = file_in.readlines()
for line in lines:
    line = line.strip()
    ids = line.split()
    pin = int(ids[3])
    chan = pin - 1
    if chan>=11: chan = chan + 1
    if chan>=22: chan = chan + 1
    if chan>=45: chan = chan + 1
    if chan>=56: chan = chan + 1
    if make_new_file:
        print(ids[0],ids[1],ids[2],chan,ids[4],ids[5], file=file_out)
    else:
        print(pin,chan)
