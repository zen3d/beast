/* BSE - Bedevilled Sound Engine
 * Copyright (C) 1997-2002 Tim Janik
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#include	"bseglobals.h"
#include	"bsemain.h"


/* --- extern variables --- */
const gdouble*	_bse_semitone_factor_table = NULL;
const gdouble*	_bse_fine_tune_factor_table = NULL;


/* --- note factors --- */
static const gdouble semitone_factor_table201[] = {
  0.00310039267962538960, /* 2^(1/12*-100) */
  0.00328475162208482243, /* 2^(1/12*-99) */
  0.00348007311773570041, /* 2^(1/12*-98) */
  0.00368700903391286522, /* 2^(1/12*-97) */
  0.00390625000000000000, /* 2^(1/12*-96) */
  0.00413852771234099713, /* 2^(1/12*-95) */
  0.00438461737620848821, /* 2^(1/12*-94) */
  0.00464534029297937917, /* 2^(1/12*-93) */
  0.00492156660115184830, /* 2^(1/12*-92) */
  0.00521421818035169674, /* 2^(1/12*-91) */
  0.00552427172801990254, /* 2^(1/12*-90) */
  0.00585276201904953711, /* 2^(1/12*-89) */
  0.00620078535925077920, /* 2^(1/12*-88) */
  0.00656950324416964487, /* 2^(1/12*-87) */
  0.00696014623547140082, /* 2^(1/12*-86) */
  0.00737401806782573044, /* 2^(1/12*-85) */
  0.00781250000000000000, /* 2^(1/12*-84) */
  0.00827705542468199426, /* 2^(1/12*-83) */
  0.00876923475241697642, /* 2^(1/12*-82) */
  0.00929068058595875834, /* 2^(1/12*-81) */
  0.00984313320230369660, /* 2^(1/12*-80) */
  0.01042843636070339348, /* 2^(1/12*-79) */
  0.01104854345603980507, /* 2^(1/12*-78) */
  0.01170552403809907420, /* 2^(1/12*-77) */
  0.01240157071850155839, /* 2^(1/12*-76) */
  0.01313900648833928974, /* 2^(1/12*-75) */
  0.01392029247094280164, /* 2^(1/12*-74) */
  0.01474803613565146089, /* 2^(1/12*-73) */
  0.01562500000000000000, /* 2^(1/12*-72) */
  0.01655411084936398850, /* 2^(1/12*-71) */
  0.01753846950483395284, /* 2^(1/12*-70) */
  0.01858136117191751667, /* 2^(1/12*-69) */
  0.01968626640460739319, /* 2^(1/12*-68) */
  0.02085687272140678695, /* 2^(1/12*-67) */
  0.02209708691207961014, /* 2^(1/12*-66) */
  0.02341104807619814842, /* 2^(1/12*-65) */
  0.02480314143700311679, /* 2^(1/12*-64) */
  0.02627801297667857947, /* 2^(1/12*-63) */
  0.02784058494188560327, /* 2^(1/12*-62) */
  0.02949607227130292177, /* 2^(1/12*-61) */
  0.03125000000000000000, /* 2^(1/12*-60) */
  0.03310822169872797702, /* 2^(1/12*-59) */
  0.03507693900966790567, /* 2^(1/12*-58) */
  0.03716272234383503334, /* 2^(1/12*-57) */
  0.03937253280921478639, /* 2^(1/12*-56) */
  0.04171374544281357390, /* 2^(1/12*-55) */
  0.04419417382415922028, /* 2^(1/12*-54) */
  0.04682209615239629684, /* 2^(1/12*-53) */
  0.04960628287400623359, /* 2^(1/12*-52) */
  0.05255602595335715894, /* 2^(1/12*-51) */
  0.05568116988377120655, /* 2^(1/12*-50) */
  0.05899214454260584354, /* 2^(1/12*-49) */
  0.06250000000000000000, /* 2^(1/12*-48) */
  0.06621644339745595404, /* 2^(1/12*-47) */
  0.07015387801933581134, /* 2^(1/12*-46) */
  0.07432544468767006667, /* 2^(1/12*-45) */
  0.07874506561842957279, /* 2^(1/12*-44) */
  0.08342749088562714780, /* 2^(1/12*-43) */
  0.08838834764831844055, /* 2^(1/12*-42) */
  0.09364419230479259368, /* 2^(1/12*-41) */
  0.09921256574801246717, /* 2^(1/12*-40) */
  0.10511205190671431788, /* 2^(1/12*-39) */
  0.11136233976754241309, /* 2^(1/12*-38) */
  0.11798428908521168708, /* 2^(1/12*-37) */
  0.12500000000000000000, /* 2^(1/12*-36) */
  0.13243288679491190807, /* 2^(1/12*-35) */
  0.14030775603867162268, /* 2^(1/12*-34) */
  0.14865088937534013334, /* 2^(1/12*-33) */
  0.15749013123685914560, /* 2^(1/12*-32) */
  0.16685498177125429560, /* 2^(1/12*-31) */
  0.17677669529663688110, /* 2^(1/12*-30) */
  0.18728838460958518735, /* 2^(1/12*-29) */
  0.19842513149602493434, /* 2^(1/12*-28) */
  0.21022410381342863576, /* 2^(1/12*-27) */
  0.22272467953508482619, /* 2^(1/12*-26) */
  0.23596857817042337416, /* 2^(1/12*-25) */
  0.25000000000000000000, /* 2^(1/12*-24) */
  0.26486577358982381614, /* 2^(1/12*-23) */
  0.28061551207734324536, /* 2^(1/12*-22) */
  0.29730177875068026668, /* 2^(1/12*-21) */
  0.31498026247371829119, /* 2^(1/12*-20) */
  0.33370996354250859121, /* 2^(1/12*-19) */
  0.35355339059327376220, /* 2^(1/12*-18) */
  0.37457676921917037470, /* 2^(1/12*-17) */
  0.39685026299204986869, /* 2^(1/12*-16) */
  0.42044820762685727152, /* 2^(1/12*-15) */
  0.44544935907016965237, /* 2^(1/12*-14) */
  0.47193715634084674832, /* 2^(1/12*-13) */
  0.50000000000000000000, /* 2^(1/12*-12) */
  0.52973154717964763228, /* 2^(1/12*-11) */
  0.56123102415468649072, /* 2^(1/12*-10) */
  0.59460355750136053336, /* 2^(1/12*-9) */
  0.62996052494743658238, /* 2^(1/12*-8) */
  0.66741992708501718242, /* 2^(1/12*-7) */
  0.70710678118654752440, /* 2^(1/12*-6) */
  0.74915353843834074940, /* 2^(1/12*-5) */
  0.79370052598409973738, /* 2^(1/12*-4) */
  0.84089641525371454303, /* 2^(1/12*-3) */
  0.89089871814033930474, /* 2^(1/12*-2) */
  0.94387431268169349664, /* 2^(1/12*-1) */
  1.00000000000000000000, /* 2^(1/12*0) */
  1.05946309435929526456, /* 2^(1/12*1) */
  1.12246204830937298143, /* 2^(1/12*2) */
  1.18920711500272106672, /* 2^(1/12*3) */
  1.25992104989487316477, /* 2^(1/12*4) */
  1.33483985417003436483, /* 2^(1/12*5) */
  1.41421356237309504880, /* 2^(1/12*6) */
  1.49830707687668149880, /* 2^(1/12*7) */
  1.58740105196819947475, /* 2^(1/12*8) */
  1.68179283050742908606, /* 2^(1/12*9) */
  1.78179743628067860948, /* 2^(1/12*10) */
  1.88774862536338699328, /* 2^(1/12*11) */
  2.00000000000000000000, /* 2^(1/12*12) */
  2.11892618871859052912, /* 2^(1/12*13) */
  2.24492409661874596287, /* 2^(1/12*14) */
  2.37841423000544213344, /* 2^(1/12*15) */
  2.51984209978974632953, /* 2^(1/12*16) */
  2.66967970834006872966, /* 2^(1/12*17) */
  2.82842712474619009760, /* 2^(1/12*18) */
  2.99661415375336299760, /* 2^(1/12*19) */
  3.17480210393639894950, /* 2^(1/12*20) */
  3.36358566101485817213, /* 2^(1/12*21) */
  3.56359487256135721896, /* 2^(1/12*22) */
  3.77549725072677398657, /* 2^(1/12*23) */
  4.00000000000000000000, /* 2^(1/12*24) */
  4.23785237743718105825, /* 2^(1/12*25) */
  4.48984819323749192574, /* 2^(1/12*26) */
  4.75682846001088426687, /* 2^(1/12*27) */
  5.03968419957949265907, /* 2^(1/12*28) */
  5.33935941668013745932, /* 2^(1/12*29) */
  5.65685424949238019521, /* 2^(1/12*30) */
  5.99322830750672599520, /* 2^(1/12*31) */
  6.34960420787279789900, /* 2^(1/12*32) */
  6.72717132202971634425, /* 2^(1/12*33) */
  7.12718974512271443792, /* 2^(1/12*34) */
  7.55099450145354797314, /* 2^(1/12*35) */
  8.00000000000000000000, /* 2^(1/12*36) */
  8.47570475487436211650, /* 2^(1/12*37) */
  8.97969638647498385147, /* 2^(1/12*38) */
  9.51365692002176853374, /* 2^(1/12*39) */
  10.07936839915898531814, /* 2^(1/12*40) */
  10.67871883336027491865, /* 2^(1/12*41) */
  11.31370849898476039041, /* 2^(1/12*42) */
  11.98645661501345199039, /* 2^(1/12*43) */
  12.69920841574559579801, /* 2^(1/12*44) */
  13.45434264405943268850, /* 2^(1/12*45) */
  14.25437949024542887584, /* 2^(1/12*46) */
  15.10198900290709594627, /* 2^(1/12*47) */
  16.00000000000000000000, /* 2^(1/12*48) */
  16.95140950974872423299, /* 2^(1/12*49) */
  17.95939277294996770294, /* 2^(1/12*50) */
  19.02731384004353706748, /* 2^(1/12*51) */
  20.15873679831797063628, /* 2^(1/12*52) */
  21.35743766672054983729, /* 2^(1/12*53) */
  22.62741699796952078083, /* 2^(1/12*54) */
  23.97291323002690398079, /* 2^(1/12*55) */
  25.39841683149119159602, /* 2^(1/12*56) */
  26.90868528811886537699, /* 2^(1/12*57) */
  28.50875898049085775169, /* 2^(1/12*58) */
  30.20397800581419189254, /* 2^(1/12*59) */
  32.00000000000000000000, /* 2^(1/12*60) */
  33.90281901949744846598, /* 2^(1/12*61) */
  35.91878554589993540587, /* 2^(1/12*62) */
  38.05462768008707413496, /* 2^(1/12*63) */
  40.31747359663594127255, /* 2^(1/12*64) */
  42.71487533344109967459, /* 2^(1/12*65) */
  45.25483399593904156165, /* 2^(1/12*66) */
  47.94582646005380796158, /* 2^(1/12*67) */
  50.79683366298238319205, /* 2^(1/12*68) */
  53.81737057623773075399, /* 2^(1/12*69) */
  57.01751796098171550338, /* 2^(1/12*70) */
  60.40795601162838378508, /* 2^(1/12*71) */
  64.00000000000000000000, /* 2^(1/12*72) */
  67.80563803899489693196, /* 2^(1/12*73) */
  71.83757109179987081175, /* 2^(1/12*74) */
  76.10925536017414826992, /* 2^(1/12*75) */
  80.63494719327188254510, /* 2^(1/12*76) */
  85.42975066688219934917, /* 2^(1/12*77) */
  90.50966799187808312331, /* 2^(1/12*78) */
  95.89165292010761592316, /* 2^(1/12*79) */
  101.59366732596476638411, /* 2^(1/12*80) */
  107.63474115247546150798, /* 2^(1/12*81) */
  114.03503592196343100675, /* 2^(1/12*82) */
  120.81591202325676757017, /* 2^(1/12*83) */
  128.00000000000000000000, /* 2^(1/12*84) */
  135.61127607798979386391, /* 2^(1/12*85) */
  143.67514218359974162349, /* 2^(1/12*86) */
  152.21851072034829653984, /* 2^(1/12*87) */
  161.26989438654376509020, /* 2^(1/12*88) */
  170.85950133376439869835, /* 2^(1/12*89) */
  181.01933598375616624662, /* 2^(1/12*90) */
  191.78330584021523184631, /* 2^(1/12*91) */
  203.18733465192953276822, /* 2^(1/12*92) */
  215.26948230495092301597, /* 2^(1/12*93) */
  228.07007184392686201350, /* 2^(1/12*94) */
  241.63182404651353514033, /* 2^(1/12*95) */
  256.00000000000000000000, /* 2^(1/12*96) */
  271.22255215597958772783, /* 2^(1/12*97) */
  287.35028436719948324698, /* 2^(1/12*98) */
  304.43702144069659307968, /* 2^(1/12*99) */
  322.53978877308753018041, /* 2^(1/12*100) */
};


/* --- fine tune factors --- */
static const gdouble fine_tune_factor_table201[] = {
  0.94387431268169349664, /* 2^(1/1200*-100) */
  0.94441967335506765930, /* 2^(1/1200*-99) */
  0.94496534913211618524, /* 2^(1/1200*-98) */
  0.94551134019490267099, /* 2^(1/1200*-97) */
  0.94605764672559590751, /* 2^(1/1200*-96) */
  0.94660426890646994096, /* 2^(1/1200*-95) */
  0.94715120691990413357, /* 2^(1/1200*-94) */
  0.94769846094838322441, /* 2^(1/1200*-93) */
  0.94824603117449739035, /* 2^(1/1200*-92) */
  0.94879391778094230692, /* 2^(1/1200*-91) */
  0.94934212095051920932, /* 2^(1/1200*-90) */
  0.94989064086613495337, /* 2^(1/1200*-89) */
  0.95043947771080207655, /* 2^(1/1200*-88) */
  0.95098863166763885907, /* 2^(1/1200*-87) */
  0.95153810291986938497, /* 2^(1/1200*-86) */
  0.95208789165082360322, /* 2^(1/1200*-85) */
  0.95263799804393738893, /* 2^(1/1200*-84) */
  0.95318842228275260453, /* 2^(1/1200*-83) */
  0.95373916455091716100, /* 2^(1/1200*-82) */
  0.95429022503218507919, /* 2^(1/1200*-81) */
  0.95484160391041655104, /* 2^(1/1200*-80) */
  0.95539330136957800103, /* 2^(1/1200*-79) */
  0.95594531759374214748, /* 2^(1/1200*-78) */
  0.95649765276708806401, /* 2^(1/1200*-77) */
  0.95705030707390124097, /* 2^(1/1200*-76) */
  0.95760328069857364694, /* 2^(1/1200*-75) */
  0.95815657382560379022, /* 2^(1/1200*-74) */
  0.95871018663959678045, /* 2^(1/1200*-73) */
  0.95926411932526439013, /* 2^(1/1200*-72) */
  0.95981837206742511631, /* 2^(1/1200*-71) */
  0.96037294505100424222, /* 2^(1/1200*-70) */
  0.96092783846103389896, /* 2^(1/1200*-69) */
  0.96148305248265312728, /* 2^(1/1200*-68) */
  0.96203858730110793932, /* 2^(1/1200*-67) */
  0.96259444310175138040, /* 2^(1/1200*-66) */
  0.96315062007004359091, /* 2^(1/1200*-65) */
  0.96370711839155186816, /* 2^(1/1200*-64) */
  0.96426393825195072828, /* 2^(1/1200*-63) */
  0.96482107983702196821, /* 2^(1/1200*-62) */
  0.96537854333265472764, /* 2^(1/1200*-61) */
  0.96593632892484555106, /* 2^(1/1200*-60) */
  0.96649443679969844984, /* 2^(1/1200*-59) */
  0.96705286714342496425, /* 2^(1/1200*-58) */
  0.96761162014234422567, /* 2^(1/1200*-57) */
  0.96817069598288301869, /* 2^(1/1200*-56) */
  0.96873009485157584337, /* 2^(1/1200*-55) */
  0.96928981693506497742, /* 2^(1/1200*-54) */
  0.96984986242010053851, /* 2^(1/1200*-53) */
  0.97041023149354054658, /* 2^(1/1200*-52) */
  0.97097092434235098615, /* 2^(1/1200*-51) */
  0.97153194115360586874, /* 2^(1/1200*-50) */
  0.97209328211448729528, /* 2^(1/1200*-49) */
  0.97265494741228551852, /* 2^(1/1200*-48) */
  0.97321693723439900559, /* 2^(1/1200*-47) */
  0.97377925176833450047, /* 2^(1/1200*-46) */
  0.97434189120170708655, /* 2^(1/1200*-45) */
  0.97490485572224024929, /* 2^(1/1200*-44) */
  0.97546814551776593878, /* 2^(1/1200*-43) */
  0.97603176077622463245, /* 2^(1/1200*-42) */
  0.97659570168566539775, /* 2^(1/1200*-41) */
  0.97715996843424595493, /* 2^(1/1200*-40) */
  0.97772456121023273979, /* 2^(1/1200*-39) */
  0.97828948020200096649, /* 2^(1/1200*-38) */
  0.97885472559803469042, /* 2^(1/1200*-37) */
  0.97942029758692687108, /* 2^(1/1200*-36) */
  0.97998619635737943501, /* 2^(1/1200*-35) */
  0.98055242209820333873, /* 2^(1/1200*-34) */
  0.98111897499831863174, /* 2^(1/1200*-33) */
  0.98168585524675451960, /* 2^(1/1200*-32) */
  0.98225306303264942693, /* 2^(1/1200*-31) */
  0.98282059854525106055, /* 2^(1/1200*-30) */
  0.98338846197391647262, /* 2^(1/1200*-29) */
  0.98395665350811212383, /* 2^(1/1200*-28) */
  0.98452517333741394660, /* 2^(1/1200*-27) */
  0.98509402165150740832, /* 2^(1/1200*-26) */
  0.98566319864018757467, /* 2^(1/1200*-25) */
  0.98623270449335917291, /* 2^(1/1200*-24) */
  0.98680253940103665527, /* 2^(1/1200*-23) */
  0.98737270355334426234, /* 2^(1/1200*-22) */
  0.98794319714051608649, /* 2^(1/1200*-21) */
  0.98851402035289613536, /* 2^(1/1200*-20) */
  0.98908517338093839536, /* 2^(1/1200*-19) */
  0.98965665641520689521, /* 2^(1/1200*-18) */
  0.99022846964637576952, /* 2^(1/1200*-17) */
  0.99080061326522932245, /* 2^(1/1200*-16) */
  0.99137308746266209128, /* 2^(1/1200*-15) */
  0.99194589242967891017, /* 2^(1/1200*-14) */
  0.99251902835739497389, /* 2^(1/1200*-13) */
  0.99309249543703590153, /* 2^(1/1200*-12) */
  0.99366629385993780037, /* 2^(1/1200*-11) */
  0.99424042381754732964, /* 2^(1/1200*-10) */
  0.99481488550142176449, /* 2^(1/1200*-9) */
  0.99538967910322905982, /* 2^(1/1200*-8) */
  0.99596480481474791428, /* 2^(1/1200*-7) */
  0.99654026282786783423, /* 2^(1/1200*-6) */
  0.99711605333458919778, /* 2^(1/1200*-5) */
  0.99769217652702331884, /* 2^(1/1200*-4) */
  0.99826863259739251122, /* 2^(1/1200*-3) */
  0.99884542173803015276, /* 2^(1/1200*-2) */
  0.99942254414138074953, /* 2^(1/1200*-1) */
  1.00000000000000000000, /* 2^(1/1200*0) */
  1.00057778950655485930, /* 2^(1/1200*1) */
  1.00115591285382360350, /* 2^(1/1200*2) */
  1.00173437023469589396, /* 2^(1/1200*3) */
  1.00231316184217284163, /* 2^(1/1200*4) */
  1.00289228786936707150, /* 2^(1/1200*5) */
  1.00347174850950278700, /* 2^(1/1200*6) */
  1.00405154395591583449, /* 2^(1/1200*7) */
  1.00463167440205376771, /* 2^(1/1200*8) */
  1.00521214004147591243, /* 2^(1/1200*9) */
  1.00579294106785343092, /* 2^(1/1200*10) */
  1.00637407767496938663, /* 2^(1/1200*11) */
  1.00695555005671880883, /* 2^(1/1200*12) */
  1.00753735840710875731, /* 2^(1/1200*13) */
  1.00811950292025838709, /* 2^(1/1200*14) */
  1.00870198379039901323, /* 2^(1/1200*15) */
  1.00928480121187417556, /* 2^(1/1200*16) */
  1.00986795537913970359, /* 2^(1/1200*17) */
  1.01045144648676378139, /* 2^(1/1200*18) */
  1.01103527472942701245, /* 2^(1/1200*19) */
  1.01161944030192248469, /* 2^(1/1200*20) */
  1.01220394339915583542, /* 2^(1/1200*21) */
  1.01278878421614531640, /* 2^(1/1200*22) */
  1.01337396294802185887, /* 2^(1/1200*23) */
  1.01395947979002913869, /* 2^(1/1200*24) */
  1.01454533493752364145, /* 2^(1/1200*25) */
  1.01513152858597472769, /* 2^(1/1200*26) */
  1.01571806093096469807, /* 2^(1/1200*27) */
  1.01630493216818885868, /* 2^(1/1200*28) */
  1.01689214249345558626, /* 2^(1/1200*29) */
  1.01747969210268639364, /* 2^(1/1200*30) */
  1.01806758119191599497, /* 2^(1/1200*31) */
  1.01865580995729237127, /* 2^(1/1200*32) */
  1.01924437859507683576, /* 2^(1/1200*33) */
  1.01983328730164409940, /* 2^(1/1200*34) */
  1.02042253627348233639, /* 2^(1/1200*35) */
  1.02101212570719324976, /* 2^(1/1200*36) */
  1.02160205579949213692, /* 2^(1/1200*37) */
  1.02219232674720795532, /* 2^(1/1200*38) */
  1.02278293874728338810, /* 2^(1/1200*39) */
  1.02337389199677490985, /* 2^(1/1200*40) */
  1.02396518669285285230, /* 2^(1/1200*41) */
  1.02455682303280147013, /* 2^(1/1200*42) */
  1.02514880121401900679, /* 2^(1/1200*43) */
  1.02574112143401776038, /* 2^(1/1200*44) */
  1.02633378389042414951, /* 2^(1/1200*45) */
  1.02692678878097877927, /* 2^(1/1200*46) */
  1.02752013630353650722, /* 2^(1/1200*47) */
  1.02811382665606650935, /* 2^(1/1200*48) */
  1.02870786003665234616, /* 2^(1/1200*49) */
  1.02930223664349202878, /* 2^(1/1200*50) */
  1.02989695667489808505, /* 2^(1/1200*51) */
  1.03049202032929762572, /* 2^(1/1200*52) */
  1.03108742780523241063, /* 2^(1/1200*53) */
  1.03168317930135891498, /* 2^(1/1200*54) */
  1.03227927501644839557, /* 2^(1/1200*55) */
  1.03287571514938695719, /* 2^(1/1200*56) */
  1.03347249989917561889, /* 2^(1/1200*57) */
  1.03406962946493038044, /* 2^(1/1200*58) */
  1.03466710404588228876, /* 2^(1/1200*59) */
  1.03526492384137750435, /* 2^(1/1200*60) */
  1.03586308905087736785, /* 2^(1/1200*61) */
  1.03646159987395846655, /* 2^(1/1200*62) */
  1.03706045651031270103, /* 2^(1/1200*63) */
  1.03765965915974735173, /* 2^(1/1200*64) */
  1.03825920802218514564, /* 2^(1/1200*65) */
  1.03885910329766432300, /* 2^(1/1200*66) */
  1.03945934518633870407, /* 2^(1/1200*67) */
  1.04005993388847775587, /* 2^(1/1200*68) */
  1.04066086960446665901, /* 2^(1/1200*69) */
  1.04126215253480637458, /* 2^(1/1200*70) */
  1.04186378288011371099, /* 2^(1/1200*71) */
  1.04246576084112139095, /* 2^(1/1200*72) */
  1.04306808661867811845, /* 2^(1/1200*73) */
  1.04367076041374864571, /* 2^(1/1200*74) */
  1.04427378242741384032, /* 2^(1/1200*75) */
  1.04487715286087075226, /* 2^(1/1200*76) */
  1.04548087191543268106, /* 2^(1/1200*77) */
  1.04608493979252924297, /* 2^(1/1200*78) */
  1.04668935669370643814, /* 2^(1/1200*79) */
  1.04729412282062671789, /* 2^(1/1200*80) */
  1.04789923837506905201, /* 2^(1/1200*81) */
  1.04850470355892899603, /* 2^(1/1200*82) */
  1.04911051857421875864, /* 2^(1/1200*83) */
  1.04971668362306726905, /* 2^(1/1200*84) */
  1.05032319890772024444, /* 2^(1/1200*85) */
  1.05093006463054025745, /* 2^(1/1200*86) */
  1.05153728099400680369, /* 2^(1/1200*87) */
  1.05214484820071636931, /* 2^(1/1200*88) */
  1.05275276645338249856, /* 2^(1/1200*89) */
  1.05336103595483586147, /* 2^(1/1200*90) */
  1.05396965690802432148, /* 2^(1/1200*91) */
  1.05457862951601300320, /* 2^(1/1200*92) */
  1.05518795398198436013, /* 2^(1/1200*93) */
  1.05579763050923824245, /* 2^(1/1200*94) */
  1.05640765930119196488, /* 2^(1/1200*95) */
  1.05701804056138037450, /* 2^(1/1200*96) */
  1.05762877449345591872, /* 2^(1/1200*97) */
  1.05823986130118871317, /* 2^(1/1200*98) */
  1.05885130118846660974, /* 2^(1/1200*99) */
  1.05946309435929526456, /* 2^(1/1200*100) */
};


/* --- functions --- */
void
bse_globals_init (void)
{
  g_return_if_fail (_bse_semitone_factor_table == NULL);
  
  /* setup semitone factorization table
   * table based on 2^(1/12*semitone)
   */
  g_assert (SFI_KAMMER_NOTE - SFI_MIN_NOTE <= 100);
  g_assert (SFI_MAX_NOTE - SFI_KAMMER_NOTE <= 100);
  _bse_semitone_factor_table = semitone_factor_table201 + 100 - SFI_KAMMER_NOTE;
  
  /* setup fine tune factorization table
   * table based on 2^(1/1200*cent)
   */
  g_assert (BSE_MIN_FINE_TUNE >= -100);
  g_assert (BSE_MAX_FINE_TUNE <= 100);
  _bse_fine_tune_factor_table = fine_tune_factor_table201 + 100;
}

gdouble
bse_db_to_factor (gdouble dB)
{
  gdouble factor = dB / 20; /* Bell */
  return pow (10, factor);
}

gdouble
bse_db_from_factor (gdouble factor,
		    gdouble min_dB)
{
  if (factor > 0)
    {
      gdouble dB = log10 (factor); /* Bell */
      dB *= 20;
      return dB;
    }
  else
    return min_dB;
}

glong
bse_time_range_to_ms (BseTimeRangeType time_range)
{
  g_return_val_if_fail (time_range >= BSE_TIME_RANGE_SHORT, 0);
  g_return_val_if_fail (time_range <= BSE_TIME_RANGE_LONG, 0);
  
  switch (time_range)
    {
    case BSE_TIME_RANGE_SHORT:		return BSE_TIME_RANGE_SHORT_ms;
    case BSE_TIME_RANGE_MEDIUM:		return BSE_TIME_RANGE_MEDIUM_ms;
    case BSE_TIME_RANGE_LONG:		return BSE_TIME_RANGE_LONG_ms;
    }
  return 0;	/* can't be triggered */
}


/* --- idle handlers --- */
/* important ordering constrains:
 * BSE_PRIORITY_HIGH		= G_PRIORITY_HIGH - 10
 * BSE_PRIORITY_NOW		= G_PRIORITY_HIGH - 5
 * G_PRIORITY_HIGH		(-100)
 * BSE_PRIORITY_NOTIFY		= G_PRIORITY_DEFAULT - 1
 * G_PRIORITY_DEFAULT		(0)
 * GDK_PRIORITY_EVENTS		= G_PRIORITY_DEFAULT
 * BSE_PRIORITY_PROG_IFACE	= G_PRIORITY_DEFAULT
 * G_PRIORITY_HIGH_IDLE		(100)
 * BSE_PRIORITY_UPDATE		= G_PRIORITY_HIGH_IDLE + 5
 * GTK_PRIORITY_RESIZE		= G_PRIORITY_HIGH_IDLE + 10
 * GDK_PRIORITY_REDRAW		= G_PRIORITY_HIGH_IDLE + 20
 * G_PRIORITY_DEFAULT_IDLE	(200)
 * G_PRIORITY_LOW		(300)
 * BSE_PRIORITY_BACKGROUND	= G_PRIORITY_LOW + 500
 */

/**
 * bse_idle_now
 * @function: user function
 * @data:     user data
 * @RETURNS:  idle handler id, suitable for bse_idle_remove()
 * Execute @function(@data) inside the main BSE thread as soon as possible.
 * This funciton is intended to be used by code which for some reason has
 * to be executed asyncronously.
 * This funciton is MT-safe and may be called from any thread.
 */
guint
bse_idle_now (GSourceFunc function,
	      gpointer    data)
{
  GSource *source = g_idle_source_new ();
  guint id;
  g_source_set_priority (source, BSE_PRIORITY_NOW);
  g_source_set_callback (source, function, data, NULL);
  id = g_source_attach (source, bse_main_context);
  g_source_unref (source);
  return id;
}

/**
 * bse_idle_notify
 * @function: user function
 * @data:     user data
 * @RETURNS:  idle handler id, suitable for bse_idle_remove()
 * Queue @function(@data) for execution inside the main BSE thread,
 * similar to bse_idle_now(), albeit with a lower priority.
 * This funciton is intended to be used by code which emits
 * asyncronous notifications.
 * This funciton is MT-safe and may be called from any thread.
 */
guint
bse_idle_notify (GSourceFunc function,
		 gpointer    data)
{
  GSource *source = g_idle_source_new ();
  guint id;
  g_source_set_priority (source, BSE_PRIORITY_NOTIFY);
  g_source_set_callback (source, function, data, NULL);
  id = g_source_attach (source, bse_main_context);
  g_source_unref (source);
  return id;
}

guint
bse_idle_normal (GSourceFunc function,
		 gpointer    data)
{
  GSource *source = g_idle_source_new ();
  guint id;
  g_source_set_priority (source, BSE_PRIORITY_NORMAL);
  g_source_set_callback (source, function, data, NULL);
  id = g_source_attach (source, bse_main_context);
  g_source_unref (source);
  return id;
}

guint
bse_idle_update (GSourceFunc function,
		 gpointer    data)
{
  GSource *source = g_idle_source_new ();
  guint id;
  g_source_set_priority (source, BSE_PRIORITY_UPDATE);
  g_source_set_callback (source, function, data, NULL);
  id = g_source_attach (source, bse_main_context);
  g_source_unref (source);
  return id;
}

guint
bse_idle_background (GSourceFunc function,
		     gpointer    data)
{
  GSource *source = g_idle_source_new ();
  guint id;
  g_source_set_priority (source, BSE_PRIORITY_BACKGROUND);
  g_source_set_callback (source, function, data, NULL);
  id = g_source_attach (source, bse_main_context);
  g_source_unref (source);
  return id;
}

/**
 * bse_idle_timed
 * @usec_delay: microsecond delay
 * @function:   user function
 * @data:       user data
 * @RETURNS:    idle handler id, suitable for bse_idle_remove()
 * Execute @function(@data) with the main BSE thread, similar to
 * bse_idle_now(), after a delay period of @usec_delay has passed.
 * This funciton is MT-safe and may be called from any thread.
 */
guint
bse_idle_timed (guint64     usec_delay,
		GSourceFunc function,
		gpointer    data)
{
  GSource *source = g_timeout_source_new (CLAMP (usec_delay / 1000, 0, G_MAXUINT));
  guint id;
  g_source_set_priority (source, BSE_PRIORITY_NOW);
  g_source_set_callback (source, function, data, NULL);
  id = g_source_attach (source, bse_main_context);
  g_source_unref (source);
  return id;
}

/**
 * bse_idle_remove
 * @id: idle handler id
 * Remove or unqueue an idle handler queued by bse_idle_now()
 * or one of its variants.
 * This funciton is MT-safe and may be called from any thread.
 */
gboolean
bse_idle_remove (guint id)
{
  GSource *source;

  g_return_val_if_fail (id > 0, FALSE);

  source = g_main_context_find_source_by_id (bse_main_context, id);
  if (source)
    g_source_destroy (source);
  return source != NULL;
}
