# [Rollover tool for taiwan 'TXF' feature price]
# This tool will detect the monthly switch day by the minimums  of no volume period of 1331~1345
# which happened in the last day of future contract.
# then to fit the delta of price to make the rollover result. (csv format)
# In the practice, it should be equivalent to let the contract expired itself and open new position next day.


import os
import struct


class CandleData:
    date_num = -1
    time_num = -1
    open_value = -1
    high_value = -1
    low_value = -1
    close_value = -1
    vol_value = -1
    rollover_delta = 0

    def __init__(self, _date_num, _time_num, _open_value, _high_value, _low_value, _close_value, _vol_value):
        self.date_num = _date_num
        self.time_num = _time_num
        self.open_value = _open_value
        self.high_value = _high_value
        self.low_value = _low_value
        self.close_value = _close_value
        self.vol_value = _vol_value
        self.rollover_delta = 0

# setup symbol here
ticker = "TXF"
bin_structure = "<iddddd"

a = os.listdir("C:\\TWFEXData\\1min\\" + ticker + "\\" + ticker + "_HOT")

outfile = "C:\\TWFEXData\\1min\\" + ticker + "\\" + ticker + "_HOT\\1min.csv"
out_fp = open(outfile, "w")

the_list = []
for i in range(len(a)):
    print(a[i])

    file_name = "C:\\TWFEXData\\1min\\" + ticker + "\\" + ticker + "_HOT\\" + a[i]
    if (not os.path.isfile(file_name)) or (file_name.find(".1min") == -1):
        continue
    fp = open(file_name, "rb")
    file_content = fp.read()
    write_buf = ""

    for j in range(len(file_content) / struct.calcsize(bin_structure)):
        decoded = struct.unpack(bin_structure, file_content[(44 * j):(44 * (j + 1))])

        date_num = a[i][0:8]
        time_num = decoded[0] / 100
        open_value = decoded[1]
        high_value = decoded[2]
        low_value = decoded[3]
        close_value = decoded[4]
        vol_value = decoded[5]

        # print(ticker+","+
        #       str(date_num) + ","+
        #       str(open_value) + "," +
        #       str(high_value) + "," +
        #       str(low_value) + "," +
        #       str(close_value) + "," +
        #       str(vol_value) + "," +
        #       str(time_num)
        #       )
        write_buf = "{0}{1}".format(write_buf, (ticker + "," +
                                                str(date_num) + "," +
                                                str(open_value) + "," +
                                                str(high_value) + "," +
                                                str(low_value) + "," +
                                                str(close_value) + "," +
                                                str(vol_value) + "," +
                                                str(time_num) + "\n"))
        the_list.append(CandleData(date_num, time_num, open_value, high_value, low_value, close_value, vol_value))

    out_fp.write(write_buf)
out_fp.close()

# rollover
delta_price = 0.0
list_count = len(the_list)
for i in range(list_count - 1, -1, -1):
    # generate new price list
    the_list[i].rollover_delta = delta_price

    # check if previous 2 min vol == 0 and this vol != 0. if yes, add new delta
    if i >= 2:
        if the_list[i - 1].vol_value == 0:
            if the_list[i - 2].vol_value == 0:
                if the_list[i].vol_value != 0:
                    delta_price = delta_price + (the_list[i].close_value - the_list[i - 1].close_value)
                    print("delta in " + str(the_list[i].date_num) + "  " + str(the_list[i].time_num) + " @ " + str(
                        the_list[i].close_value - the_list[i - 1].close_value))

# output new rollover file
outfile = "C:\\TWFEXData\\1min\\" + ticker + "\\" + ticker + "_HOT\\1min(rollover).csv"
out_fp = open(outfile, "w")
write_buf = ""

for i in range(list_count):
    current_data = the_list[i]
    write_buf = "{0}{1}".format(write_buf, (ticker + "," +
                                            str(current_data.date_num) + "," +
                                            str(current_data.open_value + current_data.rollover_delta) + "," +
                                            str(current_data.high_value + current_data.rollover_delta) + "," +
                                            str(current_data.low_value + current_data.rollover_delta) + "," +
                                            str(current_data.close_value + current_data.rollover_delta) + "," +
                                            str(current_data.vol_value) + "," +
                                            str(current_data.time_num) + "\n"))
    if len(write_buf) > 10000:
        out_fp.write(write_buf)
        write_buf = ""

out_fp.write(write_buf)
out_fp.close()
