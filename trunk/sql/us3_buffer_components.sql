--
-- us3_bufferComponent_data.sql
--
-- ported from US3 buffer component data
--

DELETE FROM bufferComponent;

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = '1-Propanol',
  viscosity     = '1.00725 -119.1062 44.35231 7.82088 -31.16201 0.0',
  density       = '0.9982 0.0 -1.21245 2.81044 -7.63148 53.39717 0-6.741 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = '2-Propanol',
  viscosity     = '1.00047 0.0 31.73283 26.30037 88.91535 0.0',
  density       = '0.9993 -12.14605 0.16192 -1.41955 0.2311 0.0 0-13.06 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Acetic acid',
  viscosity     = '0.99997 1.78527 11.71639 2.375 -0.97284 0.0',
  density       = '0.99825 -0.30226 0.8889 -0.28098 0.08607 -0.573 0-11.35 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Acetone',
  viscosity     = '1.01832 -95.42 25.016 -31.73 30 0.0',
  density       = '0.99797 1.142 -0.94445 -0.028649 6.5645 -201.13 0-1.697 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Ammonium Acetate',
  viscosity     = '1.00142 265.45 -6.225 37.27 0.0 0.0',
  density       = '0.99822 18.05 0.437 2.45 0.0 0.0 0-1 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Ammonium chloride',
  viscosity     = '1.00002 -4.07082 -2.04508 3.32166 2.22852 0.0',
  density       = '0.99824 -0.18982 1.73873 -0.92386 1.03806 -7.39899 0-4.788 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Ammonium hydroxide',
  viscosity     = '1.00034 0.0 2.17544 0.96344 -0.76054 0.0',
  density       = '0.99823 0.0 -0.75626 0.05645 0.00566 -0.05782 0-13.802 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Ammonium sulfate',
  viscosity     = '0.99864 0.0 17.77711 25.79842 100.0505 0.0',
  density       = '0.99822 0.0 7.87196 -6.4633 6.19284 -29.24905 0-3.716 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Barium chloride',
  viscosity     = '1.00031 14.91 19.463 43.9 143 0.0',
  density       = '0.99821 0.50414 17.996 -3.4269 12.895 -531.23 0-1.6 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Cadmium chloride',
  viscosity     = '1.00054 0.0 24.13067 52.14775 230.585 0.0',
  density       = '0.99837 -5.10726 16.26925 -4.81777 7.09219 -50.01997 0-2.367 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Cadmium sulfate',
  viscosity     = '1.00531 -112.3364 84.95875 -61.84851 1700.262 0.0',
  density       = '0.99794 7.86084 17.75644 16.41838 -108.4939 1884.345 0-2.531 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Calcium chloride',
  viscosity     = '1.00119 -32.316 34.63914 -26.7477 373.854 0.0',
  density       = '0.99816 1.67525 8.78941 -2.37531 1.31831 -7.86843 0-2.902 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Cesium chloride',
  viscosity     = '1.00116 -19.4104 -4.07863 11.5489 -21.774 0.0',
  density       = '0.99824 0.0 12.68641 1.27445 -11.954 258.866 0-1.916 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Citric acid',
  viscosity     = '0.99972 0.0 43.29739 104.859 811.6187 0.0',
  density       = '0.99817 0.0 7.84403 -1.8694 14.69312 -496.3454 0-1.772 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Cobaltous chloride',
  viscosity     = '0.992 71.01 26.938 116.01 91 0.0',
  density       = '0.99798 1.471 11.409 -2.9165 12.496 -0.039389 0-1.856 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Creatinine',
  viscosity     = '1.0015 -7.78 19.066 411.54 -4066.8 0.0',
  density       = '0.99908 -6.5511 4.1824 -21.007 252.35 -10365 0-721 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Cupric sulfate',
  viscosity     = '1.02133 -193.07 100.42 -258.33 3007 0.0',
  density       = '0.99827 -0.64184 16.592 -14.691 33.735 0.041691 0-1.36 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'EDTA disodium',
  viscosity     = '1.00013 0.0 98.51 0.0 21831 0.0',
  density       = '0.99824 0.0 17.82558 -16.33459 0.0 0.0 0-184 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Ethanol',
  viscosity     = '0.99491 77.6827 7.86154 62.88875 -61.69317 0.0',
  density       = '0.99818 0.73133 -1.01406 1.26963 -1.68056 5.4274 0-4.613 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Ethylene glycol',
  viscosity     = '1.0265 -115.81 24.791 -4.4436 20.079 0.0',
  density       = '0.99822 0.061852 0.74908 0.14633 -0.15139 0.11626 0-10.406 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Ferric chloride',
  viscosity     = '1.11373 -705.03 171.625 -410.68 2078.1 0.0',
  density       = '0.999 -3.3205 13.93 -11.115 27.991 -307.16 0-3.496 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Formic acid',
  viscosity     = '0.99787 10.3 2.617 -0.667075 0.391383 0.0',
  density       = '0.99873 -1.8116 1.235 -0.21354 0.0030904 0.1533 0-17.62 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Fructose',
  viscosity     = '1.00038 0.0 43.6709 104.027 916.479 0.0',
  density       = '0.99822 0.0 7.04741 -0.92356 0.69111 -8.97227 0-1.6 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Glucose',
  viscosity     = '1.00014 -67.698 65.237 -115.747 2080.655 0.0',
  density       = '0.99821 0.0 6.82008 0.19704 -4.13821 57.3788 0-1.007 M';

INSERT INTO bufferComponent SET
  units         = '%weight/weight',
  description   = 'Glycerol%',
  viscosity     = '1.0018 -8.2476 2.673 0.3109 0.1143 0.1223',
  density       = '0.99823 0.030859 0.2249 6.333e-3 2.505e-5 4.151e-5 0-32% by weight/weight';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Glycerol',
  viscosity     = '1.00176 -37.5804 27.13788 7.64333 112.0826 0.0',
  density       = '0.99824 0.0 2.08235 0.14172 -0.26453 0.54073 0-13.694 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Guanidine hydrochloride',
  viscosity     = '1.00053 -47.78951 9.78462 -13.52025 29.72242 0.0',
  density       = '0.99823 0.0 2.5887 -0.36684 0.10132 -0.20455 0-7.005 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Hydrochloric acid',
  viscosity     = '1.00061 -10.08411 6.04543 -0.35769 2.07153 0.0',
  density       = '0.99824 0.0 1.79077 -0.29517 0.16845 -0.77171 0-13.137 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Inulin',
  viscosity     = '1.01057 0.0 2177.99 841199 0.0 0.0',
  density       = '0.99867 -25.843 208.5523 0.0 0.0 0.0 0-20 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Lactic acid',
  viscosity     = '1.00482 -26.28 26.375 7.46 62.1 0.0',
  density       = '0.9994 -5.9018 2.4654 -0.79 0.45895 -1.7013 0-10.523 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Lactose',
  viscosity     = '0.98715 120.6 56.536 1613.85 -2643.2 0.0',
  density       = '0.99854 -3.9933 14.402 15.795 -1454.4 185350 0-565 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Lanthanum nitrate',
  viscosity     = '1.06922 -503.82 133.437 -586.23 4068.7 0.0',
  density       = '1.0023 -31.909 31.955 -50.5 200.02 -3482.8 0-2.043 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Lead nitrate',
  viscosity     = '1.00827 -61.35 26.067 25.09 331.4 0.0',
  density       = '0.99848 -1.475 28.361 -0.33409 -35.333 694.37 0-1.432 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Lithium chloride',
  viscosity     = '1.06331 -225.64 33.701 -33.17 49.2 0.0',
  density       = '0.9982 0.3241 2.4474 -0.75425 0.69894 -2.5406 0-8.343 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Magnesium chloride',
  viscosity     = '0.9997 7.40628 39.496 35.39141 441.972 0.0',
  density       = '0.99834 -2.52553 8.09905 -4.67138 7.16941 -51.7329 0-2.184 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Magnesium sulfate',
  viscosity     = '1.07097 -541.45 162.022 -623.45 3770.7 0.0',
  density       = '0.99757 4.3057 11.488 -4.4518 1.4447 0.2396 0-2.799 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Maltose',
  viscosity     = '1.0249 -277.99 169.59 -692.77 16953 0.0',
  density       = '0.99626 17.535 10.052 23.283 -108.96 1730.8 0-770 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Manganous sulfate',
  viscosity     = '0.95546 291.75 13.355 534.93 -15.355 0.0',
  density       = '0.99738 5.9224 13.516 2.652 -38.66 930.13 0-1.616 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Mannitol',
  viscosity     = '0.99945 51.5096 31.6108 480.648 -1093.276 0.0',
  density       = '0.99825 -0.7816 6.70635 -10.1354 74.0475 0.0 0-867 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Methanol',
  viscosity     = '1.00162 0.0 10.35988 1.42809 -3.4543 0.0',
  density       = '0.99819 0.33998 -0.61622 0.23091 -0.17744 0.2126 0-19.901 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Nickel sulfate',
  viscosity     = '0.99667 50.52 49.387 542.37 -2099.1 0.0',
  density       = '0.99463 40.585 3.8229 405.58 -9318.8 791150 0-412 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Nitric acid',
  viscosity     = '0.99847 8.53 0.43 9.32 -1.65908 0.0',
  density       = '0.99812 0.51771 3.3583 -0.26251 0.1815 -2.871 0-7.913 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Oxalic acid',
  viscosity     = '1.00379 -22.49 22.548 -37.03 148.6 0.0',
  density       = '0.9955 17.697 0.98754 45.77 -509.55 20740 0-920 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Phosphoric acid',
  viscosity     = '0.99944 -18.2304 26.885 21.8877 70.3545 0.0',
  density       = '0.99815 2.34959 4.85108 0.47553 -0.78861 0.0 0-5.117 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium bicarbonate',
  viscosity     = '0.99995 8.59665 10.17104 22.78189 7.24369 0.0',
  density       = '0.99815 0.0 6.62233 -4.52263 14.21306 -177 0-2.801 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium biphthalate',
  viscosity     = '1.00433 -30.57 51.722 -13.78 1660 0.0',
  density       = '0.99838 0.70821 7.7319 87.984 -2443.5 224850 0-405 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium bromide',
  viscosity     = '1.00215 -7.99 -4.49 9.55 2.69668 0.0',
  density       = '0.99815 0.46447 8.4543 -1.2634 2.0181 -17.792 0-4.62 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium carbonate',
  viscosity     = '1.00502 -27.95 34.01 5.11 262.7 0.0',
  density       = '0.99732 5.9523 11.484 -4.7541 2.0338 1.6067 0-2.584 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium chloride',
  viscosity     = '1.00043 0.0 -1.47276 2.98452 9.7153 0.0',
  density       = '0.99823 0.0 4.75162 -1.64923 2.56432 -20.9 0-3.742 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium chromate',
  viscosity     = '0.99631 31.42 10.3 58.63 27 0.0',
  density       = '0.99788 3.9053 14.644 -3.2244 32.244 -804.17 0-1.972 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium dichromate',
  viscosity     = '0.9999 0.0 2.89129 0.0 0.0 0.0',
  density       = '0.99827 0.0 20.35301 -15.29853 0.0 0.0 0-364 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium ferricyanide ',
  viscosity     = '1.00032 20.96 6.65 155.57 607.4 0.0',
  density       = '0.99819 0.81486 16.814 7.0814 -48.194 -3250.3 0-1.073 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium ferrocyanide',
  viscosity     = '0.99835 47.92 26.18 321.5 -37.6 0.0',
  density       = '0.99719 13.366 20.211 65.135 -1361.3 93298 0-618 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium hydroxide',
  viscosity     = '1.00011 -11.47067 11.74842 1.71339 15.48195 0.0',
  density       = '0.99838 -1.50366 4.98964 -1.6549 0.84864 -2.0378 0-13.388 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium iodide',
  viscosity     = '0.99361 46.49 -16.971 62.24 -92.8 0.0',
  density       = '0.99809 1.0762 11.986 -1.2506 3.2342 -42.415 0-2.297 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium nitrate',
  viscosity     = '1.00053 -6.64 -4.834 25.16 -18.8 0.0',
  density       = '0.99875 -2.89 6.7247 -4.7269 13.787 -174.78 0-2.759 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium oxalate',
  viscosity     = '1.00682 -64.49 31.846 -91.68 905.7 0.0',
  density       = '0.99892 -5.6914 13.027 -22.574 185.27 -5858.3 0-931 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium permanganate',
  viscosity     = '0.99627 37.23 -13.001 201.77 -2747 0.0',
  density       = '0.99757 7.9445 8.248 73.895 -2002.3 196460 0-394 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium phosphate, di-basic ',
  viscosity     = '0.99992 -6.68598 39.15859 0.0 1650.51 0.0',
  density       = '0.99826 0.0 14.74589 -16.328 112.098 0.0 0-491 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium phosphate, mono-basic',
  viscosity     = '1.00022 -14.1522 26.39116 112.6265 -458.1416 0.0',
  density       = '0.99825 0.0 9.52395 5.56057 -252.16 15765 0-786 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium sulfate',
  viscosity     = '0.99985 0.0 14.83249 170.8342 -1156.524 0.0',
  density       = '0.99823 -1.00036 14.28901 -19.07038 78.09765 0.0 0-620 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Potassium thiocyanate',
  viscosity     = '1.00307 -12.38 -4.647 15.22 1.77465 0.0',
  density       = '0.99778 2.72284 4.41185 0.0 -0.59769 3.20363 0-5.021 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Procaine hydrochloride',
  viscosity     = '1.13444 -1106.5 287.845 -1966.53 13734.2 0.0',
  density       = '0.99827 -0.051651 4.5216 4.9676 -27.695 469.02 0-1.745 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Propylene glycol',
  viscosity     = '1.007729 -331.18 56.205 -24.69 112.7 0.0',
  density       = '0.99794 1.2685 0.29384 1.4996 -2.5814 12.722 0-8.215 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Silver nitrate',
  viscosity     = '0.99789 13.77 3.314 19.13 -1.52308 0.0',
  density       = '0.99841 1.1923 13.843 0.78821 -8.4645 131.37 0-3.471 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium acetate',
  viscosity     = '1.00177 -30.89277 35.9912 44.79418 144.796 0.0',
  density       = '0.99809 4.17494 3.22595 5.42805 -18.2936 180.9158 0-2.4 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium bicarbonate',
  viscosity     = '0.99968 0.0 21.85 38.65 0.0 0.0',
  density       = '0.99823 0.0 5.96817 0.0 -114.9416 9653.325 0-743 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium bromide',
  viscosity     = '1.00206 -7.13 5.337 4.87 26.4 0.0',
  density       = '0.99806 0.9525 7.8208 -0.77671 0.49168 -1.0477 0-5.495 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium carbonate',
  viscosity     = '0.98837 58.18 42.146 157.31 851.4 0.0',
  density       = '0.99819 0.21501 11.026 -13.477 47.431 -885 0-1.638 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium chloride',
  viscosity     = '1.00193 -5.05987 9.63862 0.0 31.6583 0.0',
  density       = '0.99823 0.0 4.14712 -1.23975 1.15528 -5.21415 0-5.326 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium citrate',
  viscosity     = '0.99969 0.0 99.98432 93.8735 8944.84 0.0',
  density       = '0.9983 -4.20553 18.78849 -16.48328 0.0 763.0684 0-1.099 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium diatrizoate',
  viscosity     = '1.16872 -1681.93 539.869 -5855.61 74457.5 0.0',
  density       = '1.002 -44.591 50.903 -247.36 3568.1 -190300 0-836 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium dichromate',
  viscosity     = '1.00177 -9.25 21.66 36.37 672.8 0.0',
  density       = '0.99852 -0.80676 19.02 -5.9006 9.3529 -76.756 0-2.403 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium ferrocyanide',
  viscosity     = '0.98947 134.91 53.094 797.61 4226.5 0.0',
  density       = '0.99853 -4.0032 23.688 -109.19 2240.1 -173750 0-550 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium hydroxide',
  viscosity     = '1.00071 -27.64113 25.33124 -1.80466 109.27 0.0',
  density       = '0.99816 1.54203 4.27533 -1.521 0.6953 -1.9044 0-14.295 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium molybdate',
  viscosity     = '0.99661 50.81 34.146 268.52 -430.8 0.0',
  density       = '0.99808 3.104 15.805 86.447 -2259.3 183100 0-472 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium phosphate, di-basic',
  viscosity     = '1.00015 -26.94314 66.22344 0.0 4665.331 0.0',
  density       = '0.99826 0.0 14.15826 -28.77017 234.2374 0.0 0-408 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium phosphate, mono-basic',
  viscosity     = '0.99898 0.0 38.40933 60.6127 433.133 0.0',
  density       = '0.99826 0.0 8.83506 -3.85794 5.29888 -40.55664 0-2.625 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium phosphate, tri-basic',
  viscosity     = '0.99988 49.42915 72.27851 1062.909 0.0 0.0',
  density       = '0.99823 2.3576 18.00187 6.89182 -136.311 0.0 0-535 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium sulfate',
  viscosity     = '0.99694 0.0 40.4069 85.3196 660.247 0.0',
  density       = '0.99824 0.0 12.66872 -12.26328 64.65751 -2003.415 0-1.676 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium tartrate',
  viscosity     = '0.99371 36.82 53.736 74.55 2470.2 0.0',
  density       = '0.99801 0.96317 13.535 -12.5 43.618 -861.03 0-1.75 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium thiocyanate',
  viscosity     = '1.00329 -3.99 3.883 14.95 26.4 0.0',
  density       = '1.0017 -13.693 5.1777 -2.6498 2.6163 -11.012 0-4.643 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium thiosulfate',
  viscosity     = '1.07114 -442.34 95.362 -264.71 1266.5 0.0',
  density       = '0.99776 3.5561 12.393 -6.7155 9.5079 -91.718 0-3.498 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sodium tungstate',
  viscosity     = '1.0089 -107.33 72.684 -170.16 323.9 0.0',
  density       = '0.99583 36.856 11.497 806.13 -27379 3.4016e+06 0-332 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Strontium chloride',
  viscosity     = '1.03852 -227.77 60.16 -165.65 712.3 0.0',
  density       = '0.99827 -0.090319 13.927 -2.7774 -9.698 259.85 0-3.205 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sucrose',
  viscosity     = '1.0011 -52.13491 107.9453 0.0 11916.34 0.0',
  density       = '0.99827 -0.21963 13.23726 -1.83647 0.0 -25.13871 0-914 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Sulfuric acid',
  viscosity     = '0.99646 0.0 20.88912 0.0 43.95996 0.0',
  density       = '0.99817 2.05171 6.10052 -0.20639 -2.15038 17.65096 0-3.435 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Tartaric acid',
  viscosity     = '1.0217 -138.46 54.553 -82.49 724.1 0.0',
  density       = '0.99815 1.6109 6.4158 0.85463 -4.9431 50.272 0-2.12 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Tetracaine hydrochloride',
  viscosity     = '0.99958 0.0 82.783 1765.255 0.0 0.0',
  density       = '0.9982 0.0 3.88078 -1.06621 0.0 0.0 0-404 mM';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Trichloroacetic acid',
  viscosity     = '1.00117 -38.08678 43.68669 28.5522 16.7361 0.0',
  density       = '0.99763 0.0 36.11419 95.54537 -285.5115 4695.839 0-2.984 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Trifluoroethanol',
  viscosity     = '0.99907 -80.10114 24.86394 -16.10573 0.0 0.0',
  density       = '0.99812 3.83029 3.29838 -0.41439 0.0 0.0 0-11.276 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Tris(hydroxymethyl)aminomethane',
  viscosity     = '0.99846 0.0 32.16556 59.46483 344.514 0.0',
  density       = '0.99823 0.0 2.84717 1.31068 -3.93586 38.96285 0-2.289 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Urea',
  viscosity     = '1.00249 -19.45872 5.59969 -1.35929 6.83825 0.0',
  density       = '0.99822 0.0 1.49081 0.61729 -1.54142 10.26086 0-8.665 M';

INSERT INTO bufferComponent SET
  units         = 'mM',
  description   = 'Zinc sulfate',
  viscosity     = '0.99908 0.0 62.03292 172.7153 1177.895 0.0',
  density       = '0.99825 -0.78088 16.94782 -22.17244 142.7043 -4038.58 0-1.17 M';

