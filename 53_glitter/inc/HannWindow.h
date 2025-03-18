#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>

constexpr int16_t kMaxValue = 32767; // Maximum value for int16_t
constexpr int16_t kWindowLength = 2048; // Hann window length
constexpr int16_t kHalfHannSize = 1024; // Half Hann window length



// Hann window lookup table generated from
//  table[i] = static_cast<int16_t>(kMaxValue * 0.5 * (1 - std::cos(2 * M_PI * i / (kWindowLength-1))));

static const int16_t kHannWindowFirstHalf[kHalfHannSize] = {
0, 0, 0, 0, 1, 1, 2, 3, 4, 6, 7, 9, 11, 13, 15, 17, 
19, 22, 24, 27, 30, 34, 37, 40, 44, 48, 52, 56, 60, 64, 69, 74, 
78, 83, 89, 94, 99, 105, 111, 117, 123, 129, 135, 142, 149, 156, 163, 170, 
177, 184, 192, 200, 208, 216, 224, 232, 241, 250, 258, 267, 277, 286, 295, 305, 
315, 325, 335, 345, 355, 366, 376, 387, 398, 409, 420, 432, 443, 455, 467, 479, 
491, 503, 516, 528, 541, 554, 567, 580, 594, 607, 621, 634, 648, 663, 677, 691, 
706, 720, 735, 750, 765, 781, 796, 811, 827, 843, 859, 875, 892, 908, 925, 941, 
958, 975, 992, 1010, 1027, 1045, 1062, 1080, 1098, 1117, 1135, 1153, 1172, 1191, 1210, 1229, 
1248, 1267, 1287, 1306, 1326, 1346, 1366, 1386, 1406, 1427, 1447, 1468, 1489, 1510, 1531, 1553, 
1574, 1596, 1617, 1639, 1661, 1683, 1706, 1728, 1751, 1773, 1796, 1819, 1842, 1865, 1889, 1912, 
1936, 1960, 1984, 2008, 2032, 2056, 2081, 2105, 2130, 2155, 2180, 2205, 2230, 2256, 2281, 2307, 
2333, 2359, 2385, 2411, 2437, 2464, 2490, 2517, 2544, 2571, 2598, 2625, 2652, 2680, 2708, 2735, 
2763, 2791, 2819, 2848, 2876, 2905, 2933, 2962, 2991, 3020, 3049, 3078, 3108, 3137, 3167, 3197, 
3227, 3257, 3287, 3317, 3348, 3378, 3409, 3439, 3470, 3501, 3533, 3564, 3595, 3627, 3658, 3690, 
3722, 3754, 3786, 3818, 3850, 3883, 3916, 3948, 3981, 4014, 4047, 4080, 4113, 4147, 4180, 4214, 
4248, 4281, 4315, 4349, 4384, 4418, 4452, 4487, 4521, 4556, 4591, 4626, 4661, 4696, 4732, 4767, 
4803, 4838, 4874, 4910, 4946, 4982, 5018, 5054, 5091, 5127, 5164, 5200, 5237, 5274, 5311, 5348, 
5385, 5423, 5460, 5498, 5535, 5573, 5611, 5649, 5687, 5725, 5763, 5802, 5840, 5879, 5917, 5956, 
5995, 6034, 6073, 6112, 6151, 6191, 6230, 6269, 6309, 6349, 6389, 6428, 6468, 6509, 6549, 6589, 
6629, 6670, 6710, 6751, 6792, 6833, 6873, 6914, 6956, 6997, 7038, 7079, 7121, 7162, 7204, 7246, 
7287, 7329, 7371, 7413, 7455, 7498, 7540, 7582, 7625, 7667, 7710, 7753, 7795, 7838, 7881, 7924, 
7967, 8010, 8054, 8097, 8140, 8184, 8228, 8271, 8315, 8359, 8403, 8447, 8491, 8535, 8579, 8623, 
8667, 8712, 8756, 8801, 8846, 8890, 8935, 8980, 9025, 9070, 9115, 9160, 9205, 9250, 9296, 9341, 
9386, 9432, 9477, 9523, 9569, 9614, 9660, 9706, 9752, 9798, 9844, 9890, 9937, 9983, 10029, 10076, 
10122, 10169, 10215, 10262, 10308, 10355, 10402, 10449, 10496, 10543, 10590, 10637, 10684, 10731, 10778, 10825, 
10873, 10920, 10968, 11015, 11063, 11110, 11158, 11206, 11253, 11301, 11349, 11397, 11445, 11493, 11541, 11589, 
11637, 11685, 11733, 11781, 11830, 11878, 11926, 11975, 12023, 12072, 12120, 12169, 12218, 12266, 12315, 12364, 
12412, 12461, 12510, 12559, 12608, 12657, 12706, 12755, 12804, 12853, 12902, 12951, 13000, 13050, 13099, 13148, 
13198, 13247, 13296, 13346, 13395, 13445, 13494, 13544, 13593, 13643, 13692, 13742, 13792, 13841, 13891, 13941, 
13990, 14040, 14090, 14140, 14189, 14239, 14289, 14339, 14389, 14439, 14489, 14539, 14589, 14639, 14689, 14739, 
14789, 14839, 14889, 14939, 14989, 15039, 15089, 15140, 15190, 15240, 15290, 15340, 15390, 15441, 15491, 15541, 
15591, 15641, 15692, 15742, 15792, 15842, 15893, 15943, 15993, 16044, 16094, 16144, 16194, 16245, 16295, 16345, 
16396, 16446, 16496, 16546, 16597, 16647, 16697, 16748, 16798, 16848, 16898, 16949, 16999, 17049, 17099, 17150, 
17200, 17250, 17300, 17350, 17401, 17451, 17501, 17551, 17601, 17652, 17702, 17752, 17802, 17852, 17902, 17952, 
18002, 18052, 18102, 18152, 18202, 18252, 18302, 18352, 18402, 18452, 18502, 18552, 18601, 18651, 18701, 18751, 
18801, 18850, 18900, 18950, 18999, 19049, 19099, 19148, 19198, 19247, 19297, 19346, 19396, 19445, 19494, 19544, 
19593, 19642, 19692, 19741, 19790, 19839, 19888, 19938, 19987, 20036, 20085, 20134, 20183, 20231, 20280, 20329, 
20378, 20427, 20475, 20524, 20573, 20621, 20670, 20718, 20767, 20815, 20864, 20912, 20960, 21009, 21057, 21105, 
21153, 21201, 21249, 21297, 21345, 21393, 21441, 21489, 21537, 21584, 21632, 21680, 21727, 21775, 21822, 21870, 
21917, 21964, 22011, 22059, 22106, 22153, 22200, 22247, 22294, 22341, 22388, 22434, 22481, 22528, 22574, 22621, 
22667, 22714, 22760, 22806, 22852, 22899, 22945, 22991, 23037, 23083, 23129, 23174, 23220, 23266, 23311, 23357, 
23402, 23448, 23493, 23538, 23584, 23629, 23674, 23719, 23764, 23809, 23853, 23898, 23943, 23987, 24032, 24076, 
24121, 24165, 24209, 24253, 24297, 24341, 24385, 24429, 24473, 24517, 24560, 24604, 24647, 24691, 24734, 24777, 
24820, 24863, 24906, 24949, 24992, 25035, 25077, 25120, 25163, 25205, 25247, 25290, 25332, 25374, 25416, 25458, 
25500, 25541, 25583, 25625, 25666, 25707, 25749, 25790, 25831, 25872, 25913, 25954, 25995, 26035, 26076, 26116, 
26157, 26197, 26237, 26277, 26318, 26357, 26397, 26437, 26477, 26516, 26556, 26595, 26634, 26674, 26713, 26752, 
26791, 26829, 26868, 26907, 26945, 26984, 27022, 27060, 27098, 27136, 27174, 27212, 27249, 27287, 27325, 27362, 
27399, 27436, 27473, 27510, 27547, 27584, 27621, 27657, 27694, 27730, 27766, 27802, 27838, 27874, 27910, 27946, 
27981, 28017, 28052, 28087, 28122, 28157, 28192, 28227, 28262, 28296, 28331, 28365, 28399, 28434, 28468, 28502, 
28535, 28569, 28603, 28636, 28669, 28702, 28736, 28769, 28801, 28834, 28867, 28899, 28932, 28964, 28996, 29028, 
29060, 29092, 29124, 29155, 29187, 29218, 29249, 29280, 29311, 29342, 29373, 29403, 29434, 29464, 29494, 29524, 
29554, 29584, 29614, 29643, 29673, 29702, 29731, 29761, 29790, 29818, 29847, 29876, 29904, 29932, 29961, 29989, 
30017, 30045, 30072, 30100, 30127, 30155, 30182, 30209, 30236, 30262, 30289, 30316, 30342, 30368, 30394, 30420, 
30446, 30472, 30498, 30523, 30548, 30574, 30599, 30624, 30648, 30673, 30698, 30722, 30746, 30770, 30794, 30818, 
30842, 30866, 30889, 30912, 30935, 30958, 30981, 31004, 31027, 31049, 31072, 31094, 31116, 31138, 31160, 31181, 
31203, 31224, 31245, 31266, 31287, 31308, 31329, 31349, 31370, 31390, 31410, 31430, 31450, 31470, 31489, 31509, 
31528, 31547, 31566, 31585, 31603, 31622, 31640, 31659, 31677, 31695, 31712, 31730, 31748, 31765, 31782, 31799, 
31816, 31833, 31850, 31866, 31883, 31899, 31915, 31931, 31947, 31962, 31978, 31993, 32008, 32023, 32038, 32053, 
32068, 32082, 32096, 32111, 32125, 32138, 32152, 32166, 32179, 32192, 32206, 32219, 32231, 32244, 32257, 32269, 
32281, 32293, 32305, 32317, 32329, 32340, 32351, 32362, 32374, 32384, 32395, 32406, 32416, 32426, 32436, 32446, 
32456, 32466, 32475, 32485, 32494, 32503, 32512, 32521, 32529, 32538, 32546, 32554, 32562, 32570, 32578, 32585, 
32593, 32600, 32607, 32614, 32621, 32627, 32634, 32640, 32646, 32652, 32658, 32664, 32669, 32675, 32680, 32685, 
32690, 32695, 32699, 32704, 32708, 32712, 32716, 32720, 32724, 32727, 32731, 32734, 32737, 32740, 32743, 32745, 
32748, 32750, 32752, 32754, 32756, 32758, 32760, 32761, 32762, 32763, 32764, 32765, 32766, 32766, 32766, 32766, 
};