import re
import numpy as np


def bytes_to_c_arr(data):
    return [format(b, '#04x') for b in data]


def read_file(file_name):
    data = np.fromfile(file_name, dtype='uint8')
    data = bytearray(data)
    return data


def write_c_array(file_name, variable_name, data):
    c_file = open(file_name + ".c", "w")
    length = str(len(data))
    static_content = "const unsigned int " + variable_name + "_length = " + length + ";\n"
    static_content = static_content + "__attribute__((section (\".extflash_data\"))) const unsigned char " + variable_name + "[" + length + "] =\n"
    array_content = "{{{}}}".format(", ".join(bytes_to_c_arr(data)))
    array_content = re.sub("(.{72})", "\\1\n", array_content, 0, re.DOTALL)
    final_content = static_content + array_content
    c_file.write(final_content + ";\n")


raw_data = read_file("smw/smw_assets.dat")
write_c_array("Core/Src/porting/smw/smw_assets", "smw_assets", raw_data)
