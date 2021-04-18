import math

def main(w, sqrtW):
    # Compute which bits of `m` should be set to `1`
    m_bits = []
    for j in range(sqrtW):
        idx = w - (sqrtW - 1) - j * sqrtW + j
        m_bits.append(idx)

    # Compute the actual value of `m` with `m_bits` indices set to `1`
    m = 0
    for idx in m_bits:
        m |= 1 << idx

    print("%x" % (m))

if __name__ == "__main__":
    main(16, 4)
