Patterns tested (expected steps):
    auto pattern = "200";     // 9
    auto pattern = "2010";    // 15
    auto pattern = "20140";   // 30
    auto pattern = "2014141"; // 609
    auto pattern = "1252124"; // 2,565
    auto pattern = "1040512"; // 569,804
    auto pattern = "1042010"; // 918,339
    auto pattern = "2145142"; // 87,996,218
    auto pattern = "2014142"; // 3,563,608,205       1.5 sec (worms2x2: 10 sec)
    auto pattern = "1450224"; // 16,811,365,528       40 sec (worms2x2: 3.5 min)
    auto pattern = "1042022"; // 57,493,855,205,905   35 min (worms2x2: 3h40min)
    auto pattern = "1042020"; // 57,493,855,205,939   36 min
    auto pattern = "154411";  // infinite
    auto pattern = "12522";   // infinite hexagonal
    auto pattern = "1252121"; // > 1E80 (worms2z2: grows very fast, repeats every 18 cycles of doubling in linear size)
    auto pattern = "1525115"; // > 1E80 (worms2x2: grows very fast, repeats every 18 cycles of doubling in linear size)

    auto pattern = "1042015"; // > 1.7E22 last unknown
