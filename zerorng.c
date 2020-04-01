if (
    (lba > 0x21c0 && lba < 0x3bd800) ||
    (lba > 0x140 && lba < 0x1c0) ||
    (lba > 0x40 && lba < 0x140) ||
    (lba > 0x0 && lba < 0x40) ||
)
{
RdBfl(zero_sec);
}
