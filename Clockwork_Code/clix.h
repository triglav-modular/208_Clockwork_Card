// CLIX: the click patterns of the Electro-Harmonix 16 Second Digital Delay.
//
// A pattern is 32 steps long. Turning a pot up adds one step at a time, always in the same order,
// so there are 33 patterns, from silence to every step, each containing the one below it. At 1, 2,
// 4, 8, 16 and 32 clicks they are evenly spaced. The pot law is the 16 Second Delay's own: the
// zones that add an odd-numbered click are narrow (about 14 counts), the others 41-77 counts wide.
// Both tables come from a SPICE simulation of its CLIX circuit (EHX45000/16_Second_Delay_CLIX).

// clixRank[step]: how many steps are already clicking when this step joins.
const uint8_t clixRank[32] PROGMEM = {
     0,   16,    8,   24,    4,   20,   12,   28,
     2,   18,   10,   26,    6,   22,   14,   30,
     1,   17,    9,   25,    5,   21,   13,   29,
     3,   19,   11,   27,    7,   23,   15,   31
};

// clixThreshold[i]: the pot reading (0..1023) from which i + 1 steps click.
const uint16_t clixThreshold[32] PROGMEM = {
    14,   28,   70,   85,  135,  150,  192,  206,
   266,  281,  323,  337,  388,  403,  445,  460,
   537,  552,  594,  609,  659,  673,  715,  729,
   787,  802,  843,  857,  906,  919,  960,  974
};

// The number of steps clicking at a pot reading: the thresholds it has reached.
uint8_t clixSize(int pot) {
  uint8_t lo = 0, hi = 32;
  while (lo < hi) {
    uint8_t mid = (lo + hi) >> 1;
    if (pot >= (int)pgm_read_word_near(&clixThreshold[mid])) lo = mid + 1;
    else hi = mid;
  }
  return lo;
}

bool clixFires(int pot, int step) {
  return pgm_read_byte_near(&clixRank[step]) < clixSize(pot);
}
