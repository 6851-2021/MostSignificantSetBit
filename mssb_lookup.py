
# Find the most significant set bit for given n
def most_significant_set_bit(n):
    if n == 0:
        return -1
    else:
        return most_significant_set_bit(n >> 1) + 1


def print_lookup_table(nbits):
    lookup = ""

    lookup_name = "lookup_{}bit".format(nbits)
    max_value = 2 ** nbits

    # Open up the `lookup` array
    lookup += f"const uint32_t {lookup_name}[] = {{"

    for i in range(max_value):
        # To avoid too long lines
        if i % 16 == 0:
            lookup += "\n"

        mssb = most_significant_set_bit(i)

        if mssb == -1:
            lookup += "NO_SET_BITS"
        else:
            lookup += f"{mssb}"

        # Add commas after every element but the last
        if i < max_value - 1:
            lookup += ", "

    # Close the `lookup` array
    lookup += "};"

    print(lookup)
    

def main():
    print_lookup_table(16)

if __name__ == "__main__":
    main()
