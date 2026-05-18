# Częstotliwości Biorezonansowe

<!-- 
================================================================================
BIORESONANCE FREQUENCIES DATABASE - Machine-readable & Human-friendly Format
Arduino Nano + PoE 48V->5V Compatible
================================================================================
Format: frequency_hz|category|subcategory|description|modulation_type|carrier_khz
Categories: INJURY_BONE, INJURY_JOINT, INJURY_MUSCLE, INJURY_LIGAMENT, INJURY_NERVE, 
            INJURY_FASCIA, CIRCULATION, ANTIPARASITIC, MITOCHONDRIA, TELOMERE, DNA_REPAIR,
            ANTIBACTERIAL, ANTIVIRAL, ANTIFUNGAL, IMMUNE, HORMONAL, DETOX,
            ORGAN_SPECIFIC, PAIN, NEUROLOGICAL, RESPIRATORY, DIGESTIVE,
            CARDIOVASCULAR, LYMPHATIC, SKIN, AUTOIMMUNE, METABOLIC,
            SPIRITUALITY, VIRTUES, MERCY_ACTS, CHAKRAS, HOLY_SPIRIT_GIFTS,
            PROPHECY, LONGEVITY
Modulation: AM, FM, PWM, BURST, SWEEP, RANDOM, NONE
Carrier: carrier frequency in kHz for modulation (0 if not applicable)
ARDUINO NANO LIMITS: Max carrier 30 kHz (PWM pin 9/10), Max modulated freq 20 kHz
PoE 48V->5V: Stable 5V output for consistent frequency generation
================================================================================

FREQUENCY_DATA_START
freq_hz|category|subcategory|description|modulation|carrier_khz
--------------------------------------------------------------------------------
=== URAZY KOŚCI / BONE INJURIES ===
727|INJURY_BONE|general|Podstawowa regeneracja kości / Basic bone regeneration|AM|0
1530|INJURY_BONE|healing|Przyspieszenie zrostów kostnych / Bone union acceleration|AM|0
2128|INJURY_BONE|cellular|Stymulacja osteoblastów / Osteoblast stimulation|FM|0
3000|INJURY_BONE|acute|Gojenie złamań świeżych / Fresh fracture healing|AM|0
872|INJURY_BONE|skull|Regeneracja kości czaszki / Skull bone regeneration|FM|0
1024|INJURY_BONE|spine|Gojenie kręgów / Vertebrae healing|AM|0
1600|INJURY_BONE|marrow|Stymulacja szpiku kostnego / Bone marrow stimulation|FM|0
2500|INJURY_BONE|density|Zwiększenie gęstości kości / Bone density increase|AM|0
3500|INJURY_BONE|calcification|Wapnowanie kości / Bone calcification|FM|0
4200|INJURY_BONE|osteoporosis|Osteoporoza terapia / Osteoporosis therapy|AM|0
5000|INJURY_BONE|fracture_chronic|Przewlekłe złamania / Chronic fractures|FM|0
7270|INJURY_BONE|deep|Głęboka regeneracja kości / Deep bone regeneration|FM|10
727|INJURY_BONE|carrier_deep|Głęboka penetracja kości - FM modulacja / Deep bone penetration|FM|10
--------------------------------------------------------------------------------
=== STAWY I CHRZĄSTKI / JOINTS AND CARTILAGE ===
666|INJURY_JOINT|cartilage|Regeneracja chrząstki stawowej / Articular cartilage regeneration|AM|0
880|INJURY_JOINT|lubrication|Nawilżanie stawów / Joint lubrication|AM|0
1250|INJURY_JOINT|meniscus|Odbudowa łąkotek / Meniscus reconstruction|PWM|0
1450|INJURY_JOINT|degeneration|Leczenie zwyrodnień stawowych / Osteoarthritis treatment|AM|0
2200|INJURY_JOINT|inflammation|Redukcja stanu zapalnego w stawach / Joint inflammation reduction|AM|0
3300|INJURY_JOINT|synovial|Stymulacja produkcji mazi stawowej / Synovial fluid stimulation|FM|0
1800|INJURY_JOINT|rheumatoid|Reumatoidalne zapalenie stawów / Rheumatoid arthritis|AM|0
2400|INJURY_JOINT|gout|Dna moczanowa / Gout treatment|AM|0
2800|INJURY_JOINT|bursa|Zapalenie kaletki / Bursitis treatment|AM|0
3100|INJURY_JOINT|ankylosis|Sztywnienie stawów / Joint stiffness|FM|0
3600|INJURY_JOINT|hip|Biodro terapia / Hip therapy|PWM|8
2900|INJURY_JOINT|shoulder|Bark terapia / Shoulder therapy|AM|0
3400|INJURY_JOINT|elbow|Łokieć tenisisty / Tennis elbow|AM|0
666|INJURY_JOINT|knee_therapy|Terapia kolana / Knee therapy|AM|5
--------------------------------------------------------------------------------
=== MIĘŚNIE / MUSCLES ===
290|INJURY_MUSCLE|relax|Rozluźnienie mięśni napiętych / Muscle tension relief|AM|0
320|INJURY_MUSCLE|fiber|Regeneracja włókien mięśniowych / Muscle fiber regeneration|AM|0
415|INJURY_MUSCLE|lactic|Usuwanie kwasu mlekowego / Lactic acid removal|AM|0
528|INJURY_MUSCLE|microtear|Naprawa mikrouszkodzeń mięśni / Microtear repair|FM|0
630|INJURY_MUSCLE|elasticity|Zwiększenie elastyczności mięśni / Muscle elasticity increase|AM|0
740|INJURY_MUSCLE|strain|Leczenie naderwań mięśni / Muscle strain treatment|AM|0
850|INJURY_MUSCLE|strength|Przywracanie siły mięśniowej / Muscle strength restoration|AM|0
960|INJURY_MUSCLE|smooth|Regeneracja mięśni gładkich / Smooth muscle regeneration|FM|0
1100|INJURY_MUSCLE|cramp|Skurcze mięśni / Muscle cramps|AM|0
1250|INJURY_MUSCLE|atrophy|Zanik mięśni / Muscle atrophy|FM|0
1400|INJURY_MUSCLE|hypertrophy|Hipertrofia mięśni / Muscle hypertrophy|AM|0
1600|INJURY_MUSCLE|recovery|Regeneracja potreningowa / Post-workout recovery|AM|0
1800|INJURY_MUSCLE|fibromyalgia|Fibromialgia / Fibromyalgia|AM|0
528|INJURY_MUSCLE|deep_back|Głębokie mięśnie grzbietu / Deep back muscles|FM|12
290|INJURY_MUSCLE|quadriceps|Mięśnie czworogłowe / Quadriceps|AM|15
--------------------------------------------------------------------------------
=== WIĘZADŁA I ŚCIĘGNA / LIGAMENTS AND TENDONS ===
380|INJURY_LIGAMENT|knee_side|Gojenie więzadeł bocznych kolana / Knee collateral ligaments|AM|0
490|INJURY_LIGAMENT|achilles|Regeneracja ścięgna Achillesa / Achilles tendon regeneration|PWM|0
600|INJURY_LIGAMENT|sprain|Leczenie nadwerężeń więzadeł / Ligament sprain treatment|AM|0
710|INJURY_LIGAMENT|cruciate|Wzmacnianie więzadeł krzyżowych / Cruciate ligament strengthening|AM|0
820|INJURY_LIGAMENT|wrist|Gojenie ścięgien nadgarstka / Wrist tendon healing|AM|0
930|INJURY_LIGAMENT|rehab|Rehabilitacja po zerwaniu więzadeł / Post-tear rehabilitation|FM|0
1140|INJURY_LIGAMENT|collagen|Stymulacja kolagenu w więzadłach / Collagen stimulation|AM|0
1350|INJURY_LIGAMENT|tenosynovitis|Leczenie zapalenia pochewek ścięgnistych / Tenosynovitis|AM|0
1500|INJURY_LIGAMENT|rotator_cuff|Stożek rotatorów / Rotator cuff|AM|0
1650|INJURY_LIGAMENT|plantar_fascia|Rozciągnięcie rozcięgna podeszwowego / Plantar fascia|AM|0
1800|INJURY_LIGAMENT|patellar|Więzadło rzepki / Patellar ligament|AM|0
2000|INJURY_LIGAMENT|elbow_ucl|Więzadło łokciowe UCL / Elbow UCL|AM|0
490|INJURY_LIGAMENT|achilles_deep|Ścięgno Achillesa głębokie / Deep Achilles|PWM|7
710|INJURY_LIGAMENT|acl_pcl|Więzadła krzyżowe ACL/PCL / Cruciate ligaments|AM|9
--------------------------------------------------------------------------------
=== NERWY / NERVES ===
174|INJURY_NERVE|pain|Łagodzenie bólu nerwowego / Nerve pain relief|PWM|0
285|INJURY_NERVE|myelin|Regeneracja osłonek mielinowych / Myelin sheath regeneration|FM|0
396|INJURY_NERVE|conduction|Odblokowanie przewodnictwa nerwowego / Nerve conduction unblocking|PWM|0
507|INJURY_NERVE|neuropathy|Leczenie neuropatii obwodowych / Peripheral neuropathy|AM|0
618|INJURY_NERVE|facial|Regeneracja nerwu twarzowego / Facial nerve regeneration|AM|0
729|INJURY_NERVE|vagus|Stymulacja nerwu błędnego / Vagus nerve stimulation|PWM|0
840|INJURY_NERVE|sciatic|Leczenie rwy kulszowej / Sciatica treatment|AM|0
951|INJURY_NERVE|spinal_cord|Naprawa uszkodzeń rdzenia kręgowego / Spinal cord repair|FM|0
1062|INJURY_NERVE|cranial|Regeneracja nerwów czaszkowych / Cranial nerve regeneration|AM|0
1173|INJURY_NERVE|carpal_tunnel|Leczenie zespołu cieśni nadgarstka / Carpal tunnel syndrome|AM|0
1300|INJURY_NERVE|pinched|Ucisk nerwu / Pinched nerve|AM|0
1450|INJURY_NERVE|radial|Nerw promieniowy / Radial nerve|AM|0
1600|INJURY_NERVE|ulnar|Nerw łokciowy / Ulnar nerve|AM|0
1750|INJURY_NERVE|peroneal|Nerw strzałkowy / Peroneal nerve|AM|0
1900|INJURY_NERVE|meralgia|Meralgia paresthetica|AM|0
2100|INJURY_NERVE|autonomic|Układ nerwowy autonomiczny / Autonomic nervous system|PWM|0
285|INJURY_NERVE|deep_regen|Głęboka regeneracja nerwów / Deep nerve regeneration|FM|20
729|INJURY_NERVE|vns|Stymulacja VNS / VNS stimulation|AM|25
--------------------------------------------------------------------------------
=== POWIĘŹ / FASCIA ===
111|INJURY_FASCIA|superficial|Rozluźnienie powięzi powierzchownej / Superficial fascia release|AM|0
222|INJURY_FASCIA|trigger|Uwalnianie punktów spustowych / Trigger point release|AM|0
333|INJURY_FASCIA|deep|Regeneracja powięzi głębokiej / Deep fascia regeneration|PWM|0
444|INJURY_FASCIA|adhesion|Hydroliza zrostów powięziowych / Fascial adhesion hydrolysis|AM|0
555|INJURY_FASCIA|glide|Przywracanie ślizgu powięziowego / Fascial glide restoration|AM|0
666|INJURY_FASCIA|plantar|Leczenie zapalenia powięzi podeszwowej / Plantar fasciitis|AM|0
777|INJURY_FASCIA|chains|Rozciąganie taśm anatomicznych / Anatomical chain stretching|AM|0
888|INJURY_FASCIA|integration|Integracja strukturalna powięzi / Structural integration|AM|0
1000|INJURY_FASCIA|thoracic|Powięź piersiowa / Thoracic fascia|AM|0
1150|INJURY_FASCIA|lumbar|Powięź lędźwiowa / Lumbar fascia|AM|0
1300|INJURY_FASCIA|cervical|Powięź szyjna / Cervical fascia|AM|0
1450|INJURY_FASCIA|temporal|Powięź skroniowa / Temporal fascia|AM|0
333|INJURY_FASCIA|thoraco_lumbar|Powięź piersiowo-lędźwiowa / Thoracolumbar fascia|PWM|6
666|INJURY_FASCIA|plantar_deep|Powięź podeszwowa głęboka / Deep plantar fascia|FM|11
--------------------------------------------------------------------------------
=== UKRWIENIE I KRĄŻENIE / CIRCULATION ===
130|CIRCULATION|capillary|Rozszerzenie naczyń włosowatych / Capillary dilation|AM|0
240|CIRCULATION|micro|Poprawa mikrokrążenia / Microcirculation improvement|AM|0
350|CIRCULATION|perfusion|Zwiększenie perfuzji tkanek / Tissue perfusion increase|AM|0
460|CIRCULATION|cold_extremities|Leczenie zimnych dłoni i stóp / Cold hands and feet|AM|0
570|CIRCULATION|angiogenesis|Stymulacja angiogenezy / Angiogenesis stimulation|AM|0
680|CIRCULATION|brain|Poprawa ukrwienia mózgu / Brain circulation improvement|FM|0
790|CIRCULATION|oxygen|Zwiększenie dotlenienia mięśni / Muscle oxygenation increase|AM|0
900|CIRCULATION|ischemia|Leczenie niedokrwienia kończyn / Limb ischemia treatment|AM|0
1010|CIRCULATION|endothelium|Regeneracja śródbłonka naczyń / Endothelium regeneration|AM|0
1120|CIRCULATION|plaque|Redukcja blaszek miażdżycowych / Atherosclerotic plaque reduction|FM|0
1250|CIRCULATION|varicose|Żylaki / Varicose veins|AM|0
1400|CIRCULATION|venous|Niewydolność żylna / Venous insufficiency|AM|0
1550|CIRCULATION|arterial|Choroba tętnic obwodowych / Peripheral artery disease|AM|0
1700|CIRCULATION|raynaud|Choroba Raynauda / Raynaud's disease|AM|0
1850|CIRCULATION|claudication|Chromanie przestankowe / Intermittent claudication|AM|0
570|CIRCULATION|angiogenesis_deep|Angiogeneza terapeutyczna / Therapeutic angiogenesis|AM|18
680|CIRCULATION|brain_deep|Poprawa krążenia mózgowego / Brain circulation deep|FM|20
--------------------------------------------------------------------------------
=== ODROBACZANIE I PASOŻYTY / ANTIPARASITIC ===
20|ANTIPARASITIC|detox|Ogólna detoksykacja pasożytnicza / General parasitic detox|AM|0
125|ANTIPARASITIC|nematodes|Nicienie (glisty, owsiki) / Nematodes (roundworms, pinworms)|AM|0
250|ANTIPARASITIC|cestodes|Tasiemce / Tapeworms|AM|0
375|ANTIPARASITIC|flukes|Przywry wątrobowe / Liver flukes|AM|0
500|ANTIPARASITIC|giardia|Lamblie (Giardia) / Giardia|AM|0
625|ANTIPARASITIC|toxocara|Toksokara / Toxocara|AM|0
750|ANTIPARASITIC|filariae|Filarie / Filariae|PWM|0
875|ANTIPARASITIC|babesia|Babeszje / Babesia|AM|0
1000|ANTIPARASITIC|malaria|Plazmodium (malaria) / Plasmodium (malaria)|AM|0
1125|ANTIPARASITIC|crypto|Cryptosporidium|AM|0
1250|ANTIPARASITIC|blastocystis|Blastocystis hominis|AM|0
1375|ANTIPARASITIC|amoeba|Entamoeba histolytica|AM|0
1500|ANTIPARASITIC|protozoa|Różne protozoa / Various protozoa|AM|0
2000|ANTIPARASITIC|intestinal|Szerokie spektrum pasożytów jelitowych / Broad intestinal parasites|AM|0
2500|ANTIPARASITIC|blood|Pasożyty krwi / Blood parasites|AM|0
3000|ANTIPARASITIC|cysts|Formy przetrwalnikowe pasożytów / Parasite cyst forms|AM|0
3500|ANTIPARASITIC|larvae|Larwy pasożytów / Parasite larvae|AM|0
4000|ANTIPARASITIC|eggs|Jaja pasożytów / Parasite eggs|AM|0
4500|ANTIPARASITIC|strongyloides|Strongyloides|AM|0
5000|ANTIPARASITIC|schistosoma|Schistosoma|AM|0
5500|ANTIPARASITIC|leishmania|Leishmania|AM|0
6000|ANTIPARASITIC|trypanosoma|Trypanosoma|AM|0
125|ANTIPARASITIC|deep_tissue|Głębokie odrobaczanie tkanek / Deep tissue deworming|AM|25
750|ANTIPARASITIC|lymphatic|Filarie w układzie limfatycznym / Filariae in lymphatic|PWM|20
2000|ANTIPARASITIC|comprehensive|Kompleksowe oczyszczanie / Comprehensive cleansing|FM|15
--------------------------------------------------------------------------------
=== MITOCHONDRIA / MITOCHONDRIA ===
10|MITOCHONDRIA|sync|Synchronizacja rytmów mitochondrialnych / Mitochondrial rhythm sync|AM|0
40|MITOCHONDRIA|respiratory|Optymalizacja łańcucha oddechowego / Respiratory chain optimization|FM|0
50|MITOCHONDRIA|atp|Zwiększenie produkcji ATP / ATP production increase|AM|0
60|MITOCHONDRIA|membrane|Poprawa potencjału błonowego mitochondriów / Membrane potential|FM|0
70|MITOCHONDRIA|oxidative|Redukcja stresu oksydacyjnego / Oxidative stress reduction|AM|0
80|MITOCHONDRIA|biogenesis|Stymulacja biogenezy mitochondrialnej / Biogenesis stimulation|AM|0
90|MITOCHONDRIA|phosphorylation|Optymalizacja fosforylacji oksydacyjnej / Oxidative phosphorylation|FM|0
100|MITOCHONDRIA|density|Zwiększenie gęstości mitochondriów / Mitochondrial density increase|AM|0
110|MITOCHONDRIA|complex|Poprawa funkcji kompleksu I-IV / Complex I-IV function|FM|0
120|MITOCHONDRIA|repair|Regeneracja mitochondriów uszkodzonych / Damaged mitochondria repair|AM|0
130|MITOCHONDRIA|pgc1a|Stymulacja PGC-1α (biogeneza) / PGC-1α stimulation|PWM|0
140|MITOCHONDRIA|apoptosis|Ochrona przed apoptozą mitochondrialną / Apoptosis protection|AM|0
150|MITOCHONDRIA|dynamics|Poprawa fuzyjno-fisyjnej dynamiki / Fusion-fission dynamics|AM|0
160|MITOCHONDRIA|uncoupling|Białka rozprzęgające / Uncoupling proteins|AM|0
170|MITOCHONDRIA|ketones|Metabolizm ciał ketonowych / Ketone metabolism|AM|0
180|MITOCHONDRIA|fatigue|Zespół przewlekłego zmęczenia / Chronic fatigue syndrome|AM|0
40|MITOCHONDRIA|deep_therapy|Głęboka terapia mitochondrialna / Deep mitochondrial therapy|FM|25
80|MITOCHONDRIA|biogenesis_deep|Biogeneza mitochondrialna / Mitochondrial biogenesis|AM|20
130|MITOCHONDRIA|pgc1a_deep|Aktywacja PGC-1α / PGC-1α activation|PWM|18
--------------------------------------------------------------------------------
=== TELOMERY I DŁUGOWIECZNOŚĆ / TELOMERES AND LONGEVITY ===
144|TELOMERE|telomerase|Aktywacja telomerazy / Telomerase activation|FM|0
288|TELOMERE|lengthening|Wydłużanie telomerów / Telomere lengthening|FM|0
432|TELOMERE|dna|Harmonizacja struktury DNA / DNA structure harmonization|AM|0
576|TELOMERE|protection|Ochrona telomerów przed skracaniem / Telomere shortening protection|AM|0
720|TELOMERE|chromosome|Regeneracja końcówek chromosomów / Chromosome end regeneration|FM|0
864|TELOMERE|tert|Stymulacja ekspresji TERT / TERT expression stimulation|AM|0
1008|TELOMERE|replication|Redukcja stresu replikacyjnego / Replication stress reduction|AM|0
1152|TELOMERE|senescence|Ochrona przed senescencją komórkową / Cellular senescence protection|AM|0
1296|TELOMERE|stem|Zwiększenie aktywności telomerazy w komórkach macierzystych / Stem cell telomerase|FM|0
1440|TELOMERE|epigenetic|Odmładzanie epigenetyczne / Epigenetic rejuvenation|PWM|0
1584|TELOMERE|damage|Naprawa uszkodzeń telomerowych / Telomeric damage repair|AM|0
1728|TELOMERE|shelterin|Stabilizacja kompleksu shelterin / Shelterin complex stabilization|FM|0
1872|TELOMERE|aging|Ogólne przeciwstarzenie / General anti-aging|AM|0
2016|TELOMERE|longevity|Długowieczność / Longevity|AM|0
2160|TELOMERE|sirtuins|Aktywacja sirtuin / Sirtuin activation|AM|0
2304|TELOMERE|nad|Poziomy NAD+ / NAD+ levels|AM|0
288|TELOMERE|lengthening_deep|Wydłużanie telomerów głębokie / Deep telomere lengthening|FM|25
864|TELOMERE|tert_deep|Aktywacja TERT / TERT activation|AM|20
1440|TELOMERE|rejuvenation|Odmładzanie komórkowe / Cellular rejuvenation|PWM|18
--------------------------------------------------------------------------------
=== NAPRAWA DNA / DNA REPAIR ===
528|DNA_REPAIR|repair|Naprawa DNA / DNA repair|FM|0
639|DNA_REPAIR|healing|Uzdrowienie DNA / DNA healing|AM|0
741|DNA_REPAIR|expression|Ekspresja genów / Gene expression|AM|0
852|DNA_REPAIR|methylation|Metylacja DNA / DNA methylation|FM|0
963|DNA_REPAIR|epigenetic|Naprawa epigenetyczna / Epigenetic repair|AM|0
1074|DNA_REPAIR|mutation|Redukcja mutacji / Mutation reduction|AM|0
1185|DNA_REPAIR|strand|Naprawa nici DNA / DNA strand repair|FM|0
1296|DNA_REPAIR|double_strand|Naprawa podwójnych nici / Double strand break repair|AM|0
1407|DNA_REPAIR|oxidative|Ochrona przed uszkodzeniami oksydacyjnymi / Oxidative damage protection|AM|0
1518|DNA_REPAIR|uv|Naprawa uszkodzeń UV / UV damage repair|AM|0
1629|DNA_REPAIR|chemical|Naprawa uszkodzeń chemicznych / Chemical damage repair|AM|0
1740|DNA_REPAIR|radiation|Naprawa uszkodzeń radiacyjnych / Radiation damage repair|FM|0
1851|DNA_REPAIR|telomere_dna|Połączenie telomer-DNA / Telomere-DNA connection|AM|0
1962|DNA_REPAIR|chromosome|Stabilizacja chromosomów / Chromosome stabilization|AM|0
2073|DNA_REPAIR|histone|Modyfikacja histonów / Histone modification|AM|0
528|DNA_REPAIR|love_frequency|Częstotliwość miłości - naprawa DNA / Love frequency DNA repair|FM|15
639|DNA_REPAIR|miracle|Cudowna naprawa DNA / Miracle DNA repair|AM|20
--------------------------------------------------------------------------------
=== DUCHOWOŚĆ I ROZWÓJ / SPIRITUALITY AND DEVELOPMENT ===
96|SPIRITUALITY|meditation|Głęboka medytacja / Deep meditation|AM|0
144|SPIRITUALITY|prayer|Modlitwa i kontemplacja / Prayer and contemplation|AM|0
192|SPIRITUALITY|mindfulness|Uważność / Mindfulness|AM|0
240|SPIRITUALITY|compassion|Współczucie / Compassion|AM|0
288|SPIRITUALITY|forgiveness|Przebaczenie / Forgiveness|FM|0
336|SPIRITUALITY|gratitude|Wdzięczność / Gratitude|AM|0
384|SPIRITUALITY|humility|Pokora / Humility|AM|0
432|SPIRITUALITY|universal_harmony|Harmonia uniwersalna / Universal harmony|FM|0
480|SPIRITUALITY|inner_peace|Wewnętrzny pokój / Inner peace|AM|0
528|SPIRITUALITY|transformation|Transformacja / Transformation|FM|0
576|SPIRITUALITY|enlightenment|Oświecenie / Enlightenment|AM|0
624|SPIRITUALITY|divine_connection|Połączenie z boskością / Divine connection|AM|0
672|SPIRITUALITY|soul_healing|Uzdrowienie duszy / Soul healing|AM|0
720|SPIRITUALITY|higher_self|Wyższe Ja / Higher self|FM|0
768|SPIRITUALITY|ascension|Wniebowstąpienie / Ascension|AM|0
816|SPIRITUALITY|unity|Jedność / Unity|AM|0
864|SPIRITUALITY|cosmic_consciousness|Kosmiczna świadomość / Cosmic consciousness|FM|0
912|SPIRITUALITY|divine_love|Boska miłość / Divine love|AM|0
960|SPIRITUALITY|spiritual_awakening|Przebudzenie duchowe / Spiritual awakening|AM|0
1008|SPIRITUALITY|third_eye|Trzecie oko / Third eye|PWM|0
1056|SPIRITUALITY|crown_chakra|Czakra korony / Crown chakra|FM|0
1104|SPIRITUALITY|pineal_activation|Aktywacja szyszynki / Pineal activation|PWM|0
1152|SPIRITUALITY|mystical_experience|Doświadczenie mistyczne / Mystical experience|AM|0
1200|SPIRITUALITY|samadhi|Samadhi|AM|0
144|SPIRITUALITY|deep_prayer|Głęboka modlitwa / Deep prayer|FM|18
432|SPIRITUALITY|cosmic_harmony|Kosmiczna harmonia / Cosmic harmony|FM|20
528|SPIRITUALITY|dna_spiritual|Duchowa naprawa DNA / Spiritual DNA repair|FM|25
--------------------------------------------------------------------------------
=== CZNOTY KARDYNALNE / CARDINAL VIRTUES ===
111|VIRTUES|prudence|Roztropność / Prudence|AM|0
222|VIRTUES|justice|Sprawiedliwość / Justice|AM|0
333|VIRTUES|fortitude|Męstwo / Fortitude|AM|0
444|VIRTUES|temperance|Umiarkowanie / Temperance|AM|0
555|VIRTUES|faith|Wiara / Faith|FM|0
666|VIRTUES|hope|Nadzieja / Hope|AM|0
777|VIRTUES|charity|Miłość / Charity (Love)|FM|0
888|VIRTUES|wisdom|Mądrość / Wisdom|AM|0
999|VIRTUES|understanding|Rozumienie / Understanding|AM|0
1110|VIRTUES|counsel|Rada / Counsel|AM|0
1221|VIRTUES|knowledge|Wiedza / Knowledge|AM|0
1332|VIRTUES|piety|Pobożność / Piety|FM|0
1443|VIRTUES|fear_of_lord|Bojaźń Boża / Fear of the Lord|AM|0
1554|VIRTUES|patience|Cierpliwość / Patience|AM|0
1665|VIRTUES|kindness|Łagodność / Kindness|AM|0
1776|VIRTUES|goodness|Dobroć / Goodness|AM|0
1887|VIRTUES|faithfulness|Wierność / Faithfulness|AM|0
1998|VIRTUES|gentleness|Cichość / Gentleness|AM|0
2109|VIRTUES|self_control|Opanowanie / Self-control|AM|0
111|VIRTUES|prudence_deep|Roztropność głęboka / Deep prudence|AM|15
333|VIRTUES|fortitude_deep|Męstwo głębokie / Deep fortitude|AM|18
777|VIRTUES|charity_deep|Miłość głęboka / Deep charity|FM|20
--------------------------------------------------------------------------------
=== AKTY MIŁOSIERDZIA / ACTS OF MERCY ===
108|MERCY_ACTS|feed_hungry|Nakarmić głodnych / Feed the hungry|AM|0
216|MERCY_ACTS|give_drink|Dać pić spragnionym / Give drink to the thirsty|AM|0
324|MERCY_ACTS|clothe_naked|Odziać nagich / Clothe the naked|AM|0
432|MERCY_ACTS|shelter_homeless|Przyjąć w gościnę / Shelter the homeless|AM|0
540|MERCY_ACTS|visit_sick|Odwiedzić chorych / Visit the sick|FM|0
648|MERCY_ACTS|visit_prisoners|Odwiedzić więzionych / Visit prisoners|AM|0
756|MERCY_ACTS|bury_dead|Pogrzebać umarłych / Bury the dead|AM|0
864|MERCY_ACTS|counsel_doubtful|Pouczyć nieumiejących / Counsel the doubtful|AM|0
972|MERCY_ACTS|instruct_ignorant|Instruować niewiedzących / Instruct the ignorant|FM|0
1080|MERCY_ACTS|admonish_sinner|Upominać grzeszących / Admonish sinners|AM|0
1188|MERCY_ACTS|comfort_sorrowful|Pocieszać strapionych / Comfort the sorrowful|AM|0
1296|MERCY_ACTS|forgive_injuries|Przebaczyć krzywdy / Forgive injuries|FM|0
1404|MERCY_ACTS|bear_wrongs|Znosić cudze złe / Bear wrongs patiently|AM|0
1512|MERCY_ACTS|pray_living|Modlić się za żywych / Pray for the living|AM|0
1620|MERCY_ACTS|pray_dead|Modlić się za zmarłych / Pray for the dead|AM|0
540|MERCY_ACTS|healing_deep|Głębokie uzdrowienie chorych / Deep healing of sick|FM|22
756|MERCY_ACTS|mercy_comprehensive|Kompleksowe akty miłosierdzia / Comprehensive mercy acts|AM|25
--------------------------------------------------------------------------------
=== CZAKRY / CHAKRAS ===
194|CHAKRAS|root_muladhara|Czakra podstawy / Root chakra Muladhara|AM|0
288|CHAKRAS|sacral_svadhishthana|Czakra sakralna / Sacral chakra Svadhishthana|AM|0
384|CHAKRAS|solar_plexus_manipura|Czakra splotu słonecznego / Solar plexus Manipura|AM|0
480|CHAKRAS|heart_anahata|Czakra serca / Heart chakra Anahata|FM|0
576|CHAKRAS|throat_vishuddha|Czakra gardła / Throat chakra Vishuddha|AM|0
672|CHAKRAS|third_eye_ajna|Czakra trzeciego oka / Third eye Ajna|PWM|0
768|CHAKRAS|crown_sahasrara|Czakra korony / Crown chakra Sahasrara|FM|0
864|CHAKRAS|earth_stellar|Czakra gwiezdnej ziemi / Earth star chakra|AM|0
960|CHAKRAS|soul_star|Czakra gwiazdy duszy / Soul star chakra|AM|0
1056|CHAKRAS|causal|Czakra przyczynowa / Causal chakra|AM|0
1152|CHAKRAS|celestial|Czakra niebiańska / Celestial chakra|AM|0
1248|CHAKRAS|divine|Czakra boska / Divine chakra|FM|0
1344|CHAKRAS|golden|Złota czakra / Golden chakra|AM|0
1440|CHAKRAS|crystal|Kryształowa czakra / Crystal chakra|AM|0
1536|CHAKRAS|plasma|Plazmowa czakra / Plasma chakra|AM|0
1632|CHAKRAS|unity|Czakra jedności / Unity chakra|AM|0
194|CHAKRAS|root_balance|Balans czakry podstawy / Root chakra balance|AM|12
480|CHAKRAS|heart_healing|Uzdrowienie czakry serca / Heart chakra healing|FM|18
768|CHAKRAS|crown_activation|Aktywacja czakry korony / Crown chakra activation|FM|25
--------------------------------------------------------------------------------
=== DARY DUCHA ŚWIĘTEGO / GIFTS OF THE HOLY SPIRIT ===
144|HOLY_SPIRIT_GIFTS|wisdom|Mądrość / Wisdom|FM|0
288|HOLY_SPIRIT_GIFTS|understanding|Rozum / Understanding|AM|0
432|HOLY_SPIRIT_GIFTS|counsel|Rada / Counsel|AM|0
576|HOLY_SPIRIT_GIFTS|fortitude|Męstwo / Fortitude|AM|0
720|HOLY_SPIRIT_GIFTS|knowledge|Wiedza / Knowledge|FM|0
864|HOLY_SPIRIT_GIFTS|piety|Pobożność / Piety|AM|0
1008|HOLY_SPIRIT_GIFTS|fear_of_lord|Bojaźń Boża / Fear of the Lord|AM|0
1152|HOLY_SPIRIT_GIFTS|prophecy|Proroctwo / Prophecy|PWM|0
1296|HOLY_SPIRIT_GIFTS|healing|Uzdrowienie / Healing|FM|0
1440|HOLY_SPIRIT_GIFTS|miracles|Cuda / Miracles|AM|0
1584|HOLY_SPIRIT_GIFTS|discernment|Rozróżnianie duchów / Discernment of spirits|AM|0
1728|HOLY_SPIRIT_GIFTS|tongues|Dar języków / Speaking in tongues|PWM|0
1872|HOLY_SPIRIT_GIFTS|interpretation|Interpretacja języków / Interpretation of tongues|AM|0
2016|HOLY_SPIRIT_GIFTS|apostleship|Apostołowanie / Apostleship|AM|0
2160|HOLY_SPIRIT_GIFTS|teaching|Nauczanie / Teaching|AM|0
2304|HOLY_SPIRIT_GIFTS|exhortation|Napominanie / Exhortation|AM|0
2448|HOLY_SPIRIT_GIFTS|giving|Szczodrość / Giving|AM|0
2592|HOLY_SPIRIT_GIFTS|leadership|Przewodzenie / Leadership|AM|0
2736|HOLY_SPIRIT_GIFTS|mercy|Miłosierdzie / Mercy|FM|0
144|HOLY_SPIRIT_GIFTS|wisdom_deep|Głęboka mądrość / Deep wisdom|FM|20
720|HOLY_SPIRIT_GIFTS|knowledge_deep|Głęboka wiedza / Deep knowledge|FM|25
--------------------------------------------------------------------------------
=== WIZJE PROROCZE I DAR PROROCTWA / PROPHETIC VISIONS AND PROPHECY ===
111|PROPHECY|activation|Aktywacja daru proroctwa / Prophecy gift activation|PWM|0
222|PROPHECY|clarity|Jasnowidzenie / Clairvoyance|AM|0
333|PROPHECY|clairaudience|Jasnosłyszenie / Clairaudience|AM|0
444|PROPHECY|clairsentience|Jasnoczucie / Clairsentience|FM|0
555|PROPHECY|dreams|Sny prorocze / Prophetic dreams|AM|0
666|PROPHECY|visions|Wizje / Visions|PWM|0
777|PROPHECY|discernment|Rozróżnianie / Discernment|AM|0
888|PROPHECY|word_knowledge|Słowo wiedzy / Word of knowledge|AM|0
999|PROPHECY|word_wisdom|Słowo mądrości / Word of wisdom|FM|0
1110|PROPHECY|future_sight|Widzenie przyszłości / Future sight|AM|0
1221|PROPHECY|remote_viewing|Widzenie na odległość / Remote viewing|AM|0
1332|PROPHECY|astral_projection|Projekcja astralna / Astral projection|PWM|0
1443|PROPHECY|lucid_dreaming|Świadome śnienie / Lucid dreaming|AM|0
1554|PROPHECY|intuition|Intuicja / Intuition|AM|0
1665|PROPHECY|psychic_protection|Ochrona psychiczna / Psychic protection|AM|0
1776|PROPHECY|spiritual_shield|Tarcza duchowa / Spiritual shield|AM|0
1887|PROPHECY|angelic_communication|Komunikacja z aniołami / Angelic communication|FM|0
1998|PROPHECY|divine_messages|Boskie przesłania / Divine messages|AM|0
2109|PROPHECY|biblical_prophecy|Proroctwa biblijne / Biblical prophecy|AM|0
2220|PROPHECY|end_times|Proroctwa czasów ostatecznych / End times prophecy|AM|0
222|PROPHECY|clarity_deep|Głębokie jasnowidzenie / Deep clairvoyance|AM|18
666|PROPHECY|visions_deep|Głębokie wizje / Deep visions|PWM|22
888|PROPHECY|word_deep|Głębokie słowo wiedzy / Deep word of knowledge|FM|25
--------------------------------------------------------------------------------
=== DŁUGOWIECZNOŚĆ I WYDŁUŻENIE ŻYCIA / LONGEVITY AND LIFE EXTENSION ===
100|LONGEVITY|cellular_renewal|Odnawianie komórkowe / Cellular renewal|AM|0
200|LONGEVITY|stem_cells|Aktywacja komórek macierzystych / Stem cell activation|FM|0
300|LONGEVITY|autophagy|Autofagia / Autophagy|AM|0
400|LONGEVITY|senolytics|Usuwanie komórek starzejących się / Senolytics|AM|0
500|LONGEVITY|nad_boost|Zwiększenie NAD+ / NAD+ boost|AM|0
600|LONGEVITY|sirtuin_activation|Aktywacja sirtuin / Sirtuin activation|FM|0
700|LONGEVITY|ampk|Aktywacja AMPK / AMPK activation|AM|0
800|LONGEVITY|mtor_inhibition|Inhibicja mTOR / mTOR inhibition|AM|0
900|LONGEVITY|foxo|Aktywacja FOXO / FOXO activation|AM|0
1000|LONGEVITY|klotho|Białko Klotho / Klotho protein|FM|0
1100|LONGEVITY|telomerase_full|Pełna aktywacja telomerazy / Full telomerase activation|FM|0
1200|LONGEVITY|mitohormesis|Mitohormeza / Mitohormesis|AM|0
1300|LONGEVITY|caloric_mimic|Symulacja restrykcji kalorycznych / Caloric restriction mimic|AM|0
1400|LONGEVITY|rapamycin_freq|Efekt rapamycyny częstotliwościowej / Rapamycin frequency effect|AM|0
1500|LONGEVITY|metformin_freq|Efekt metforminy częstotliwościowej / Metformin frequency effect|AM|0
1600|LONGEVITY|resveratrol|Efekt resweratrolu / Resveratrol effect|FM|0
1700|LONGEVITY|quercetin|Efekt kwercetyny / Quercetin effect|AM|0
1800|LONGEVITY|fisetin|Efekt fisetyny / Fisetin effect|AM|0
1900|LONGEVITY|sproutlin|Efekt sprolutyny / Sproutlin effect|AM|0
2000|LONGEVITY|young_blood|Efekt młodej krwi / Young blood effect|FM|0
2100|LONGEVITY|parabiosis|Parabioza częstotliwościowa / Frequency parabiosis|AM|0
2200|LONGEVITY|organ_regeneration|Regeneracja organów / Organ regeneration|AM|0
2300|LONGEVITY|tissue_rejuvenation|Odmładzanie tkanek / Tissue rejuvenation|FM|0
2400|LONGEVITY|skin_young|Odmłodzenie skóry / Skin rejuvenation|AM|0
2500|LONGEVITY|hair_regrowth|Odrastanie włosów / Hair regrowth|AM|0
2600|LONGEVITY|vision_restore|Przywrócenie wzroku / Vision restoration|AM|0
2700|LONGEVITY|hearing_restore|Przywrócenie słuchu / Hearing restoration|AM|0
2800|LONGEVITY|memory_enhance|Poprawa pamięci / Memory enhancement|FM|0
2900|LONGEVITY|cognitive_youth|Kognitywna młodzieńczość / Cognitive youth|AM|0
3000|LONGEVITY|physical_youth|Fizyczna młodzieńczość / Physical youth|AM|0
100|LONGEVITY|renewal_deep|Głębokie odnawianie / Deep cellular renewal|AM|20
500|LONGEVITY|nad_deep|Głęboki boost NAD+ / Deep NAD+ boost|FM|25
1100|LONGEVITY|telomerase_max|Maksymalna telomeraza / Maximum telomerase|FM|30
--------------------------------------------------------------------------------
=== PRZECIWBAKTERYJNE / ANTIBACTERIAL ===
727|ANTIBACTERIAL|ecoli|E. coli|AM|0
787|ANTIBACTERIAL|staph|Staphylococcus aureus|AM|0
880|ANTIBACTERIAL|strep|Streptococcus|AM|0
1550|ANTIBACTERIAL|salmonella|Salmonella|AM|0
2128|ANTIBACTERIAL|pseudomonas|Pseudomonas aeruginosa|FM|0
3000|ANTIBACTERIAL|hpylori|Helicobacter pylori|AM|0
4500|ANTIBACTERIAL|tb|Mycobacterium tuberculosis|FM|0
6000|ANTIBACTERIAL|cdiff|Clostridium difficile|AM|0
7500|ANTIBACTERIAL|legionella|Legionella pneumophila|AM|0
9000|ANTIBACTERIAL|lyme|Borrelia burgdorferi (Lyme)|FM|0
10500|ANTIBACTERIAL|bartonella|Bartonella|AM|0
12000|ANTIBACTERIAL|babesia_microti|Babesia microti|AM|0
13500|ANTIBACTERIAL|ehrlichia|Ehrlichia|AM|0
15000|ANTIBACTERIAL|mycoplasma|Mycoplasma|FM|0
16500|ANTIBACTERIAL|chlamydia|Chlamydia|AM|0
18000|ANTIBACTERIAL|klebsiella|Klebsiella|AM|0
19500|ANTIBACTERIAL|enterobacter|Enterobacter|AM|0
21000|ANTIBACTERIAL|proteus|Proteus|AM|0
--------------------------------------------------------------------------------
=== PRZECIWWIRUSOWE / ANTIVIRAL ===
125|ANTIVIRAL|hsv|Herpes simplex|AM|0
250|ANTIVIRAL|ebv|EBV (Epstein-Barr)|AM|0
375|ANTIVIRAL|cmv|CMV (cytomegalowirus)|AM|0
500|ANTIVIRAL|hpv|HPV|AM|0
625|ANTIVIRAL|hiv|HIV (terapia wspomagająca) / HIV supportive|AM|0
750|ANTIVIRAL|hcv|HCV (wirusowe zapalenie wątroby C) / Hepatitis C|AM|0
875|ANTIVIRAL|hbv|HBV (wirusowe zapalenie wątroby B) / Hepatitis B|AM|0
1000|ANTIVIRAL|influenza|Influenza|AM|0
1125|ANTIVIRAL|rsv|RSV|AM|0
1250|ANTIVIRAL|adenovirus|Adenowirusy / Adenoviruses|AM|0
1375|ANTIVIRAL|enterovirus|Enterowirusy / Enteroviruses|AM|0
1500|ANTIVIRAL|parvovirus|Parwowirusy / Parvoviruses|AM|0
2000|ANTIVIRAL|coronavirus|Koronawirusy / Coronaviruses|AM|0
2500|ANTIVIRAL|rna_viruses|Wirusy RNA ogólnie / RNA viruses general|AM|0
3000|ANTIVIRAL|dna_viruses|Wirusy DNA ogólnie / DNA viruses general|FM|0
3500|ANTIVIRAL|retrovirus|Retrowirusy / Retroviruses|FM|0
4000|ANTIVIRAL|orthomyxovirus|Orthomyxoviruses|AM|0
4500|ANTIVIRAL|paramyxovirus|Paramyxoviruses|AM|0
5000|ANTIVIRAL|picornavirus|Pikornawirusy / Picornaviruses|AM|0
5500|ANTIVIRAL|flavivirus|Flawiwirusy / Flaviviruses|AM|0
6000|ANTIVIRAL|togavirus|Togawirusy / Togaviruses|AM|0
--------------------------------------------------------------------------------
=== PRZECIWGRZYBICZE / ANTIFUNGAL ===
350|ANTIFUNGAL|candida_albicans|Candida albicans|AM|0
700|ANTIFUNGAL|candida_glabrata|Candida glabrata|AM|0
1050|ANTIFUNGAL|aspergillus|Aspergillus|AM|0
1400|ANTIFUNGAL|cryptococcus|Cryptococcus|FM|0
1750|ANTIFUNGAL|trichophyton|Trichophyton (grzybica skóry) / Skin fungus|AM|0
2100|ANTIFUNGAL|microsporum|Microsporum|AM|0
2450|ANTIFUNGAL|blastomyces|Blastomyces|AM|0
2800|ANTIFUNGAL|histoplasma|Histoplasma|AM|0
3150|ANTIFUNGAL|pneumocystis|Pneumocystis|AM|0
3500|ANTIFUNGAL|molds|Pleśnie różne / Various molds|AM|0
3850|ANTIFUNGAL|sporothrix|Sporothrix|AM|0
4200|ANTIFUNGAL|coccidioides|Coccidioides|AM|0
4550|ANTIFUNGAL|paracoccidioides|Paracoccidioides|AM|0
4900|ANTIFUNGAL|penicillium|Penicillium|AM|0
5250|ANTIFUNGAL|rhizopus|Rhizopus|AM|0
5600|ANTIFUNGAL|mucor|Mucor|AM|0
--------------------------------------------------------------------------------
=== UKŁAD ODPORNOŚCIOWY / IMMUNE SYSTEM ===
100|IMMUNE|t_cells|Stymulacja limfocytów T / T-cell stimulation|AM|0
200|IMMUNE|b_cells|Aktywacja limfocytów B / B-cell activation|AM|0
300|IMMUNE|antibodies|Zwiększenie produkcji przeciwciał / Antibody production increase|AM|0
400|IMMUNE|macrophages|Stymulacja makrofagów / Macrophage stimulation|AM|0
500|IMMUNE|nk_cells|Aktywacja komórek NK / NK cell activation|AM|0
600|IMMUNE|proinflammatory|Regulacja cytokin prozapalnych / Pro-inflammatory cytokines|AM|0
700|IMMUNE|antiinflammatory|Redukcja cytokin przeciwzapalnych / Anti-inflammatory cytokines|AM|0
800|IMMUNE|th1_th2|Balans Th1/Th2 / Th1/Th2 balance|AM|0
900|IMMUNE|thymus|Stymulacja grasicy / Thymus stimulation|AM|0
1000|IMMUNE|spleen|Regeneracja śledziony / Spleen regeneration|AM|0
1100|IMMUNE|complement|Aktywacja układu dopełniacza / Complement system activation|AM|0
1200|IMMUNE|modulation|Modulacja odpowiedzi immunologicznej / Immune response modulation|AM|0
1300|IMMUNE|autoimmune|Choroby autoimmunologiczne / Autoimmune diseases|AM|0
1400|IMMUNE|allergies|Alergie / Allergies|AM|0
1500|IMMUNE|inflammation|Stan zapalny ogólny / General inflammation|AM|0
1600|IMMUNE|interferon|Interferony / Interferons|AM|0
1700|IMMUNE|interleukins|Interleukiny / Interleukins|AM|0
--------------------------------------------------------------------------------
=== UKŁAD HORMONALNY / HORMONAL SYSTEM ===
50|HORMONAL|pineal|Szyszynka i melatonina / Pineal gland and melatonin|AM|0
100|HORMONAL|pituitary|Przysadka mózgowa / Pituitary gland|AM|0
150|HORMONAL|thyroid|Tarczyca (T3, T4) / Thyroid (T3, T4)|AM|0
200|HORMONAL|parathyroid|Przytarczyce (PTH) / Parathyroid (PTH)|AM|0
250|HORMONAL|adrenals|Nadnercza (kortyzol, adrenalina) / Adrenals (cortisol, adrenaline)|AM|0
300|HORMONAL|pancreas|Trzustka (insulina, glukagon) / Pancreas (insulin, glucagon)|AM|0
350|HORMONAL|ovaries|Jajniki (estrogen, progesteron) / Ovaries (estrogen, progesterone)|AM|0
400|HORMONAL|testes|Jądra (testosteron) / Testes (testosterone)|AM|0
450|HORMONAL|gh|Hormon wzrostu (GH) / Growth hormone|AM|0
500|HORMONAL|prolactin|Prolaktyna / Prolactin|AM|0
550|HORMONAL|oxytocin|Oksytocyna / Oxytocin|AM|0
600|HORMONAL|adh|Wazopresyna (ADH) / Vasopressin (ADH)|AM|0
650|HORMONAL|aldosterone|Aldosteron / Aldosterone|AM|0
700|HORMONAL|dhea|DHEA|AM|0
750|HORMONAL|pregnenolone|Pregnenolon / Pregnenolone|AM|0
800|HORMONAL|cortisol|Kortyzol / Cortisol|AM|0
850|HORMONAL|insulin_resistance|Insulinooporność / Insulin resistance|AM|0
900|HORMONAL|hashimoto|Hashimoto|AM|0
950|HORMONAL|graves|Choroba Gravesa-Basedowa / Graves' disease|AM|0
1000|HORMONAL|pcos|PCOS (zespół policystycznych jajników)|AM|0
1050|HORMONAL|endometriosis|Endometrioza / Endometriosis|AM|0
--------------------------------------------------------------------------------
=== DETOKSYKACJA NARZĄDOWA / ORGAN DETOX ===
80|DETOX|liver_phase1|Wątroba - faza I detoksu / Liver phase I detox|AM|0
160|DETOX|liver_phase2|Wątroba - faza II detoksu / Liver phase II detox|AM|0
240|DETOX|liver_phase3|Wątroba - faza III detoksu / Liver phase III detox|AM|0
320|DETOX|kidney_filter|Nerki - filtracja kłębuszkowa / Kidney glomerular filtration|AM|0
400|DETOX|kidney_reabsorb|Nerki - reabsorpcja kanalikowa / Kidney tubular reabsorption|AM|0
480|DETOX|intestines|Jelita - perystaltyka / Intestines peristalsis|AM|0
560|DETOX|lungs|Płuca - drenaż limfatyczny / Lungs lymphatic drainage|AM|0
640|DETOX|skin|Skóra - eliminacja przez pot / Skin sweat elimination|AM|0
720|DETOX|lymphatic|Układ limfatyczny - drenaż / Lymphatic system drainage|AM|0
800|DETOX|spleen|Śledziona - filtracja krwi / Spleen blood filtration|AM|0
880|DETOX|gallbladder|Woreczek żółciowy - przepływ żółci / Gallbladder bile flow|AM|0
960|DETOX|pancreas_enzymes|Trzustka - enzymy trawienne / Pancreas digestive enzymes|AM|0
1040|DETOX|colon|Jelito grubego - oczyszczanie / Colon cleansing|AM|0
1120|DETOX|heavy_metals|Metale ciężkie / Heavy metals|AM|0
1200|DETOX|fluoride|Fluorki / Fluorides|AM|0
1280|DETOX|pesticides|Pestycydy / Pesticides|AM|0
1360|DETOX|radiation|Promieniowanie / Radiation|AM|0
1440|DETOX|chemotherapy|Chemioterapia - wsparcie detoksu / Chemotherapy detox support|AM|0
--------------------------------------------------------------------------------
=== CHOROBY UKŁADU ODDECHOWEGO / RESPIRATORY DISEASES ===
200|RESPIRATORY|asthma|Astma / Asthma|AM|0
350|RESPIRATORY|bronchitis|Zapalenie oskrzeli / Bronchitis|AM|0
500|RESPIRATORY|pneumonia|Zapalenie płuc / Pneumonia|AM|0
650|RESPIRATORY|copd|POChP / COPD|AM|0
800|RESPIRATORY|emphysema|Rozedma płuc / Emphysema|AM|0
950|RESPIRATORY|pulmonary_fibrosis|Zwłóknienie płuc / Pulmonary fibrosis|AM|0
1100|RESPIRATORY|sinusitis|Zapalenie zatok / Sinusitis|AM|0
1250|RESPIRATORY|rhinitis|Nieżyt nosa / Rhinitis|AM|0
1400|RESPIRATORY|sleep_apnea|Bezdech senny / Sleep apnea|AM|0
1550|RESPIRATORY|cough|Kaszel przewlekły / Chronic cough|AM|0
1700|RESPIRATORY|pleurisy|Zapalenie opłucnej / Pleurisy|AM|0
1850|RESPIRATORY|tuberculosis|Gruźlica płuc / Pulmonary tuberculosis|FM|0
--------------------------------------------------------------------------------
=== CHOROBY UKŁADU TRAWIENNEGO / DIGESTIVE DISEASES ===
150|DIGESTIVE|ibs|Zespół jelita drażliwego / Irritable bowel syndrome|AM|0
300|DIGESTIVE|crohn|Choroba Leśniowskiego-Crohna / Crohn's disease|AM|0
450|DIGESTIVE|colitis|Wrzodziejące zapalenie jelita grubego / Ulcerative colitis|AM|0
600|DIGESTIVE|celiac|Celiakia / Celiac disease|AM|0
750|DIGESTIVE|gerd|Refluks żołądkowo-przełykowy / GERD|AM|0
900|DIGESTIVE|ulcers|Wrzody żołądka / Stomach ulcers|AM|0
1050|DIGESTIVE|diverticulitis|Diverticulitis|AM|0
1200|DIGESTIVE|hemorrhoids|Hemoroidy / Hemorrhoids|AM|0
1350|DIGESTIVE|constipation|Zaparcia / Constipation|AM|0
1500|DIGESTIVE|diarrhea|Biegunki / Diarrhea|AM|0
1650|DIGESTIVE|malabsorption|Zespół złego wchłaniania / Malabsorption syndrome|AM|0
1800|DIGESTIVE|sibo|SIBO (przerost bakteryjny jelita cienkiego)|AM|0
--------------------------------------------------------------------------------
=== CHOROBY SERCA I NACZYŃ / CARDIOVASCULAR DISEASES ===
100|CARDIOVASCULAR|hypertension|Nadciśnienie / Hypertension|AM|0
250|CARDIOVASCULAR|hypotension|Niedociśnienie / Hypotension|AM|0
400|CARDIOVASCULAR|arrhythmia|Arytmia / Arrhythmia|AM|0
550|CARDIOVASCULAR|tachycardia|Tachykardia / Tachycardia|AM|0
700|CARDIOVASCULAR|bradycardia|Bradykardia / Bradycardia|AM|0
850|CARDIOVASCULAR|angina|Dławica piersiowa / Angina|AM|0
1000|CARDIOVASCULAR|heart_failure|Niewydolność serca / Heart failure|AM|0
1150|CARDIOVASCULAR|mi|Zawał serca - rehabilitacja / Myocardial infarction rehab|AM|0
1300|CARDIOVASCULAR|stroke|Udar mózgu - rehabilitacja / Stroke rehab|AM|0
1450|CARDIOVASCULAR|atherosclerosis|Miażdżyca / Atherosclerosis|FM|0
1600|CARDIOVASCULAR|dvt|Żylna zakrzepica głęboka / Deep vein thrombosis|AM|0
1750|CARDIOVASCULAR|aneurysm|Tętniak / Aneurysm|AM|0
--------------------------------------------------------------------------------
=== CHOROBY SKÓRY / SKIN DISEASES ===
200|SKIN|eczema|Egzema / Eczema|AM|0
400|SKIN|psoriasis|Łuszczyca / Psoriasis|AM|0
600|SKIN|acne|Trądzik / Acne|AM|0
800|SKIN|rosacea|Trądzik różowaty / Rosacea|AM|0
1000|SKIN|dermatitis|Zapalenie skóry / Dermatitis|AM|0
1200|SKIN|vitiligo|Bielactwo / Vitiligo|AM|0
1400|SKIN|alopecia|Łysienie / Alopecia|AM|0
1600|SKIN|warts|Brodawki / Warts|AM|0
1800|SKIN|herpes_skin|Opryszczka skórna / Herpes skin|AM|0
2000|SKIN|fungal_skin|Grzybica skóry / Skin fungus|AM|0
2200|SKIN|cellulitis|Cellulitis|AM|0
2400|SKIN|scars|Blizny / Scars|AM|0
2600|SKIN|burns|Oparzenia / Burns|AM|0
2800|SKIN|wound_healing|Gojenie ran / Wound healing|AM|0
--------------------------------------------------------------------------------
=== CHOROBY AUTOIMMUNOLOGICZNE / AUTOIMMUNE DISEASES ===
150|AUTOIMMUNE|ms|Stwardnienie rozsiane / Multiple sclerosis|AM|0
350|AUTOIMMUNE|ra|Reumatoidalne zapalenie stawów / Rheumatoid arthritis|AM|0
550|AUTOIMMUNE|lupus|Toczeń rumieniowaty układowy / Lupus|AM|0
750|AUTOIMMUNE|type1_diabetes|Cukrzyca typu 1 / Type 1 diabetes|AM|0
950|AUTOIMMUNE|hashimotos|Zapalenie tarczycy Hashimoto / Hashimoto's thyroiditis|AM|0
1150|AUTOIMMUNE|graves|Choroba Gravesa-Basedowa / Graves' disease|AM|0
1350|AUTOIMMUNE|sjogren|Zespół Sjögrena / Sjögren's syndrome|AM|0
1550|AUTOIMMUNE|scleroderma|Twardzina / Scleroderma|AM|0
1750|AUTOIMMUNE|polymyositis|Polimiozitis|AM|0
1950|AUTOIMMUNE|dermatomyositis|Dermatomyositis|AM|0
2150|AUTOIMMUNE|vasculitis|Zapalenie naczyń / Vasculitis|AM|0
2350|AUTOIMMUNE|addison|Choroba Addisona / Addison's disease|AM|0
--------------------------------------------------------------------------------
=== CHOROBY METABOLICZNE / METABOLIC DISEASES ===
100|METABOLIC|obesity|Otyłość / Obesity|AM|0
300|METABOLIC|diabetes_type2|Cukrzyca typu 2 / Type 2 diabetes|AM|0
500|METABOLIC|metabolic_syndrome|Zespół metaboliczny / Metabolic syndrome|AM|0
700|METABOLIC|high_cholesterol|Wysoki cholesterol / High cholesterol|AM|0
900|METABOLIC|high_triglycerides|Wysokie trójglicerydy / High triglycerides|AM|0
1100|METABOLIC|gout|Dna moczanowa / Gout|AM|0
1300|METABOLIC|fatty_liver|Stłuszczenie wątroby / Fatty liver|AM|0
1500|METABOLIC|hypothyroidism|Niedoczynność tarczycy / Hypothyroidism|AM|0
1700|METABOLIC|hyperthyroidism|Nadczynność tarczycy / Hyperthyroidism|AM|0
1900|METABOLIC|adrenal_fatigue|Zmęczenie nadnerczy / Adrenal fatigue|AM|0
2100|METABOLIC|leptin_resistance|Oporność na leptynę / Leptin resistance|AM|0
--------------------------------------------------------------------------------
=== BÓL I PRZECIWZAPALNE / PAIN AND ANTI-INFLAMMATORY ===
111|PAIN|general|Ból ogólny / General pain|AM|0
222|PAIN|acute|Ból ostry / Acute pain|AM|0
333|PAIN|chronic|Ból przewlekły / Chronic pain|AM|0
444|PAIN|inflammatory|Ból zapalny / Inflammatory pain|AM|0
555|PAIN|neuropathic|Ból neuropatyczny / Neuropathic pain|PWM|0
666|PAIN|joint|Ból stawów / Joint pain|AM|0
777|PAIN|muscle|Ból mięśni / Muscle pain|AM|0
888|PAIN|back|Ból pleców / Back pain|AM|0
999|PAIN|neck|Ból szyi / Neck pain|AM|0
1111|PAIN|headache|Ból głowy / Headache|AM|0
1222|PAIN|migraine|Migrena / Migraine|AM|0
1333|PAIN|fibromyalgia|Fibromialgia / Fibromyalgia|AM|0
1444|PAIN|arthritis|Artretyzm / Arthritis|AM|0
1555|PAIN|post_surgical|Ból pooperacyjny / Post-surgical pain|AM|0
1666|PAIN|cancer|Ból nowotworowy / Cancer pain|AM|0
--------------------------------------------------------------------------------
=== CHOROBY NEUROLOGICZNE / NEUROLOGICAL DISEASES ===
100|NEUROLOGICAL|alzheimers|Alzheimer|AM|0
300|NEUROLOGICAL|parkinsons|Parkinson|AM|0
500|NEUROLOGICAL|epilepsy|Padaczka / Epilepsy|AM|0
700|NEUROLOGICAL|als|ALS (stwardnienie zanikowe boczne)|AM|0
900|NEUROLOGICAL|huntingtons|Choroba Huntingtona / Huntington's disease|AM|0
1100|NEUROLOGICAL|dementia|Otępienie / Dementia|AM|0
1300|NEUROLOGICAL|neuropathy|Neuropatia obwodowa / Peripheral neuropathy|AM|0
1500|NEUROLOGICAL|restless_leg|Zespół niespokojnych nóg / Restless leg syndrome|AM|0
1700|NEUROLOGICAL|vertigo|Zawroty głowy / Vertigo|AM|0
1900|NEUROLOGICAL|bell_palsy|Porażenie Bella / Bell's palsy|AM|0
2100|NEUROLOGICAL|trigeminal|Nerwoból nerwu trójdzielnego / Trigeminal neuralgia|AM|0
2300|NEUROLOGICAL|post_polio|Zespół post-polio / Post-polio syndrome|AM|0
--------------------------------------------------------------------------------
=== UKŁAD LIMFATYCZNY / LYMPHATIC SYSTEM ===
150|LYMPHATIC|drainage|Drenaż limfatyczny / Lymphatic drainage|AM|0
350|LYMPHATIC|swelling|Obrzęk limfatyczny / Lymphedema|AM|0
550|LYMPHATIC|nodes|Węzły chłonne / Lymph nodes|AM|0
750|LYMPHATIC|vessels|Naczynia limfatyczne / Lymphatic vessels|AM|0
950|LYMPHATIC|spleen_support|Wsparcie śledziony / Spleen support|AM|0
1150|LYMPHATIC|tonsils|Migdałki / Tonsils|AM|0
1350|LYMPHATIC|adenoids|Adenoidy / Adenoids|AM|0
1550|LYMPHATIC|thymus_support|Wsparcie grasicy / Thymus support|AM|0
--------------------------------------------------------------------------------
=== CZĘSTOTLIWOŚCI NOŚNE / CARRIER FREQUENCIES (Arduino Nano Compatible: 1-30 kHz) ===
1|CARRIER|surface|Nośna 1 kHz - terapia powierzchniowa / Surface therapy carrier|NONE|1
2|CARRIER|shallow|Nośna 2 kHz - tkanki płytkie / Shallow tissue carrier|NONE|2
3|CARRIER|skin|Nośna 3 kHz - skóra i tkanka podskórna / Skin and subcutaneous carrier|NONE|3
5|CARRIER|muscle|Nośna 5 kHz - mięśnie powierzchowne / Superficial muscles carrier|NONE|5
7|CARRIER|medium|Nośna 7 kHz - tkanki średnio-głębokie / Medium-depth tissue carrier|NONE|7
10|CARRIER|deep|Nośna 10 kHz - tkanki głębokie / Deep tissue carrier|NONE|10
12|CARRIER|organs|Nośna 12 kHz - organy wewnętrzne / Internal organs carrier|NONE|12
15|CARRIER|cns|Nośna 15 kHz - układ nerwowy centralny / CNS carrier|NONE|15
18|CARRIER|bone|Nośna 18 kHz - kości i stawy / Bone and joint carrier|NONE|18
20|CARRIER|marrow|Nośna 20 kHz - szpik kostny / Bone marrow carrier|NONE|20
25|CARRIER|brain|Nośna 25 kHz - mózg przez czaszkę / Brain through skull carrier|NONE|25
30|CARRIER|max_arduino|Maksymalna dla Arduino Nano 30 kHz / Max for Arduino Nano|NONE|30
--------------------------------------------------------------------------------
=== MODULACJE / MODULATION TYPES ===
0|MODULATION|am|AM (Amplitude Modulation) - najlepsza dla tkanki miękkiej / Best for soft tissue|AM|0
0|MODULATION|fm|FM (Frequency Modulation) - najlepsza dla tkanki twardej (kości) / Best for hard tissue (bone)|FM|0
0|MODULATION|pwm|PWM (Pulse Width Modulation) - najlepsza dla układu nerwowego / Best for nervous system|PWM|0
0|MODULATION|burst|Burst Mode - serie impulsów dla maksymalnej absorpcji / Pulse bursts for max absorption|BURST|0
0|MODULATION|sweep|Sweep Mode - przemiatanie częstotliwości dla uniknięcia adaptacji / Frequency sweep to avoid adaptation|SWEEP|0
0|MODULATION|random|Random Mode - losowe zmiany dla zapobiegania tolerancji / Random changes to prevent tolerance|RANDOM|0
--------------------------------------------------------------------------------
=== INTERFERENCJA / INTERFERENCE COMBINATIONS ===
100|INTERFERENCE|deep_penetration|4000 Hz + 4100 Hz = 100 Hz częstotliwość różnicowa (głęboka penetracja)|AM|0
250|INTERFERENCE|combo_250|5000 Hz + 5250 Hz = 250 Hz częstotliwość różnicowa|AM|0
500|INTERFERENCE|combo_500|6000 Hz + 6500 Hz = 500 Hz częstotliwość różnicowa|AM|0
727|INTERFERENCE|bone|8000 Hz + 8727 Hz = 727 Hz częstotliwość różnicowa (kość)|AM|0
528|INTERFERENCE|dna|10000 Hz + 10528 Hz = 528 Hz częstotliwość różnicowa (naprawa DNA)|AM|0
--------------------------------------------------------------------------------
=== CZĘSTOTLIWOŚCI W kHz / FREQUENCIES IN KILOHERTZ ===
=== CHOROBY NOWOTWOROWE / CANCER THERAPY ===
10.5|CANCER|general|Ogólna terapia nowotworowa / General cancer therapy|AM|0
15.75|CANCER|breast|Rak piersi / Breast cancer|FM|0
21.28|CANCER|lung|Rak płuc / Lung cancer|AM|0
27.5|CANCER|prostate|Rak prostaty / Prostate cancer|FM|0
33.0|CANCER|colon|Rak jelita grubego / Colon cancer|AM|0
42.5|CANCER|pancreas|Rak trzustki / Pancreatic cancer|FM|0
48.75|CANCER|liver|Rak wątroby / Liver cancer|AM|0
55.0|CANCER|brain|Guz mózgu / Brain tumor|FM|0
62.5|CANCER|leukemia|Białaczka / Leukemia|AM|0
70.0|CANCER|lymphoma|Chłoniak / Lymphoma|FM|0
78.5|CANCER|ovarian|Rak jajnika / Ovarian cancer|AM|0
85.0|CANCER|kidney|Rak nerki / Kidney cancer|FM|0
92.5|CANCER|bladder|Rak pęcherza / Bladder cancer|AM|0
100.0|CANCER|melanoma|Czerniak / Melanoma|FM|0
10.5|CANCER|deep_tissue|Głęboka terapia nowotworowa / Deep cancer therapy|AM|150
21.28|CANCER|metastasis|Przerzuty / Metastases|FM|175
--------------------------------------------------------------------------------
=== CHOROBY SERCA I UKŁADU KRĄŻENIA / CARDIOVASCULAR DISEASES ===
5.0|CARDIOVASCULAR|arrhythmia|Arytmia serca / Heart arrhythmia|AM|0
8.5|CARDIOVASCULAR|hypertension|Nadciśnienie tętnicze / Hypertension|FM|0
12.0|CARDIOVASCULAR|angina|Dławica piersiowa / Angina pectoris|AM|0
16.5|CARDIOVASCULAR|heart_failure|Niewydolność serca / Heart failure|FM|0
22.0|CARDIOVASCULAR|atherosclerosis|Miażdżyca / Atherosclerosis|AM|0
28.5|CARDIOVASCULAR|myocardial|Infarkt mięśnia sercowego / Myocardial infarction|FM|0
35.0|CARDIOVASCULAR|valve|Wady zastawek / Valve defects|AM|0
42.0|CARDIOVASCULAR|pericarditis|Zapalenie osierdzia / Pericarditis|FM|0
50.0|CARDIOVASCULAR|endocarditis|Zapalenie wsierdzia / Endocarditis|AM|0
58.0|CARDIOVASCULAR|cardiomyopathy|Kardiomiopatia / Cardiomyopathy|FM|0
66.0|CARDIOVASCULAR|palpitations|Kołatanie serca / Heart palpitations|AM|0
75.0|CARDIOVASCULAR|edema|Obrzęk płuc / Pulmonary edema|FM|0
8.5|CARDIOVASCULAR|deep_therapy|Głęboka terapia kardiologiczna / Deep cardiac therapy|AM|120
--------------------------------------------------------------------------------
=== CHOROBY UKŁADU ODDECHOWEGO / RESPIRATORY DISEASES ===
3.5|RESPIRATORY|asthma|Astma oskrzelowa / Bronchial asthma|AM|0
6.0|RESPIRATORY|bronchitis|Zapalenie oskrzeli / Bronchitis|FM|0
9.5|RESPIRATORY|pneumonia|Zapalenie płuc / Pneumonia|AM|0
13.0|RESPIRATORY|copd|POChP / COPD|FM|0
17.5|RESPIRATORY|emphysema|Rozedma płuc / Emphysema|AM|0
23.0|RESPIRATORY|fibrosis|Zwłóknienie płuc / Pulmonary fibrosis|FM|0
30.0|RESPIRATORY|pleurisy|Zapalenie opłucnej / Pleurisy|AM|0
38.0|RESPIRATORY|tuberculosis|Gruźlica / Tuberculosis|FM|0
47.0|RESPIRATORY|sleep_apnea|Bezdech senny / Sleep apnea|AM|0
56.0|RESPIRATORY|cystic_fibrosis|Mukowiscydoza / Cystic fibrosis|FM|0
65.0|RESPIRATORY|pulmonary_embolism|Zatorowość płucna / Pulmonary embolism|AM|0
75.0|RESPIRATORY|silicosis|Pylica płuc / Silicosis|FM|0
9.5|RESPIRATORY|deep_lung|Głęboka terapia płucna / Deep lung therapy|AM|100
--------------------------------------------------------------------------------
=== CHOROBY UKŁADU POKARMOWEGO / DIGESTIVE DISEASES ===
4.0|DIGESTIVE|gastritis|Zapalenie żołądka / Gastritis|AM|0
7.5|DIGESTIVE|ulcer|Wrzody żołądka i dwunastnicy / Stomach and duodenal ulcers|FM|0
11.0|DIGESTIVE|reflux|Refluks żołądkowo-przełykowy / GERD|AM|0
15.5|DIGESTIVE|crohn|Choroba Leśniowskiego-Crohna / Crohn's disease|FM|0
21.0|DIGESTIVE|colitis|Wrzodziejące zapalenie jelita grubego / Ulcerative colitis|AM|0
28.0|DIGESTIVE|ibs|Zespół jelita drażliwego / IBS|FM|0
36.0|DIGESTIVE|hepatitis|Zapalenie wątroby / Hepatitis|AM|0
45.0|DIGESTIVE|cirrhosis|Marskość wątroby / Liver cirrhosis|FM|0
55.0|DIGESTIVE|gallstones|Kamica żółciowa / Gallstones|AM|0
66.0|DIGESTIVE|pancreatitis|Zapalenie trzustki / Pancreatitis|FM|0
78.0|DIGESTIVE|diverticulitis|Uchyłkowatość jelit / Diverticulitis|AM|0
90.0|DIGESTIVE|celiac|Choroba trzewna / Celiac disease|FM|0
11.0|DIGESTIVE|liver_detox|Detoksykacja wątroby / Liver detox|AM|130
--------------------------------------------------------------------------------
=== CHOROBY NEREK I DRÓG MOCZOWYCH / KIDNEY AND URINARY DISEASES ===
5.5|URINARY|kidney_stones|Kamica nerkowa / Kidney stones|AM|0
9.0|URINARY|nephritis|Zapalenie nerek / Nephritis|FM|0
13.5|URINARY|uti|Infekcje dróg moczowych / UTI|AM|0
19.0|URINARY|renal_failure|Niewydolność nerek / Renal failure|FM|0
26.0|URINARY|pyelonephritis|Odmiedniczkowe zapalenie nerek / Pyelonephritis|AM|0
34.0|URINARY|glomerulonephritis|Kłębuszkowe zapalenie nerek / Glomerulonephritis|FM|0
43.0|URINARY|incontinence|Nietrzymanie moczu / Urinary incontinence|AM|0
53.0|URINARY|interstitial_cystitis|Śródmiąższowe zapalenie pęcherza / Interstitial cystitis|FM|0
64.0|URINARY|enlarged_prostate|Powiększenie prostaty / Enlarged prostate|AM|0
76.0|URINARY|urethritis|Zapalenie cewki moczowej / Urethritis|FM|0
88.0|URINARY|bladder_infection|Infekcja pęcherza / Bladder infection|AM|0
13.5|URINARY|kidney_clean|Oczyszczanie nerek / Kidney cleansing|FM|110
--------------------------------------------------------------------------------
=== CHOROBY SKÓRY / SKIN DISEASES ===
2.5|SKIN|acne|Trądzik / Acne|AM|0
4.5|SKIN|eczema|Egzema / Eczema|FM|0
7.0|SKIN|psoriasis|Łuszczyca / Psoriasis|AM|0
10.0|SKIN|dermatitis|Zapalenie skóry / Dermatitis|FM|0
14.0|SKIN|rosacea|Trądzik różowaty / Rosacea|AM|0
19.0|SKIN|vitiligo|Bielactwo / Vitiligo|FM|0
25.0|SKIN|herpes|Opryszczka / Herpes|AM|0
32.0|SKIN|shingles|Półpasiec / Shingles|FM|0
40.0|SKIN|warts|Brodawki / Warts|AM|0
49.0|SKIN|fungal|Infekcje grzybicze / Fungal infections|FM|0
59.0|SKIN|cellulite|Cellulit / Cellulite|AM|0
70.0|SKIN|scars|Blizny / Scars|FM|0
82.0|SKIN|burns|Oparzenia / Burns|AM|0
95.0|SKIN|aging|Starzenie się skóry / Skin aging|FM|0
7.0|SKIN|deep_regeneration|Głęboka regeneracja skóry / Deep skin regeneration|AM|85
--------------------------------------------------------------------------------
=== CHOROBY AUTOIMMUNOLOGICZNE / AUTOIMMUNE DISEASES ===
6.5|AUTOIMMUNE|lupus|Toczeń rumieniowaty / Lupus erythematosus|AM|0
10.5|AUTOIMMUNE|rheumatoid_arthritis|Reumatoidalne zapalenie stawów / Rheumatoid arthritis|FM|0
15.0|AUTOIMMUNE|multiple_sclerosis|Stwardnienie rozsiane / Multiple sclerosis|AM|0
20.5|AUTOIMMUNE|hashimoto|Choroba Hashimoto / Hashimoto's thyroiditis|FM|0
27.0|AUTOIMMUNE|grave|Choroba Gravesa-Basedowa / Graves' disease|AM|0
35.0|AUTOIMMUNE|type1_diabetes|Cukrzyca typu 1 / Type 1 diabetes|FM|0
44.0|AUTOIMMUNE|sjogren|Zespół Sjögrena / Sjögren's syndrome|AM|0
54.0|AUTOIMMUNE|scleroderma|Twardzina układowa / Systemic sclerosis|FM|0
65.0|AUTOIMMUNE|polymyositis|Polimiozapalenie / Polymyositis|AM|0
77.0|AUTOIMMUNE|vasculitis|Zapalenie naczyń / Vasculitis|FM|0
90.0|AUTOIMMUNE|myasthenia_gravis|Miastenia / Myasthenia gravis|AM|0
104.0|AUTOIMMUNE|ankylosing_spondylitis|Zesztywniające zapalenie stawów kręgosłupa / Ankylosing spondylitis|FM|0
15.0|AUTOIMMUNE|immune_balance|Regulacja układu odpornościowego / Immune system regulation|AM|140
--------------------------------------------------------------------------------
=== CHOROBY NEUROLOGICZNE / NEUROLOGICAL DISEASES ===
3.0|NEUROLOGICAL|migraine|Migrena / Migraine|AM|0
5.5|NEUROLOGICAL|epilepsy|Padaczka / Epilepsy|FM|0
8.5|NEUROLOGICAL|parkinson|Choroba Parkinsona / Parkinson's disease|AM|0
12.5|NEUROLOGICAL|alzheimer|Choroba Alzheimera / Alzheimer's disease|FM|0
17.0|NEUROLOGICAL|als|Stwardnienie zanikowe boczne / ALS|AM|0
23.0|NEUROLOGICAL|neuropathy|Neuropatia obwodowa / Peripheral neuropathy|FM|0
30.0|NEUROLOGICAL|stroke|Udar mózgu / Stroke recovery|AM|0
38.0|NEUROLOGICAL|vertigo|Zawroty głowy / Vertigo|FM|0
47.0|NEUROLOGICAL|essential_tremor|Drżenie samoistne / Essential tremor|AM|0
57.0|NEUROLOGICAL|restless_leg|Zespół niespokojnych nóg / Restless leg syndrome|FM|0
68.0|NEUROLOGICAL|bell_palsy|Porażenie Bella / Bell's palsy|AM|0
80.0|NEUROLOGICAL|trigeminal|Neuralgia nerwu trójdzielnego / Trigeminal neuralgia|FM|0
93.0|NEUROLOGICAL|optic_neuritis|Zapalenie nerwu wzrokowego / Optic neuritis|AM|0
107.0|NEUROLOGICAL|guillain_barre|Zespół Guillaina-Barrégo / Guillain-Barré syndrome|FM|0
8.5|NEUROLOGICAL|brain_regen|Regeneracja neuronalna / Neuronal regeneration|AM|160
--------------------------------------------------------------------------------
=== CHOROBY ENDOKRYNOLOGICZNE / ENDOCRINE DISEASES ===
4.5|HORMONAL|hypothyroid|Niedoczynność tarczycy / Hypothyroidism|AM|0
7.5|HORMONAL|hyperthyroid|Nadczynność tarczycy / Hyperthyroidism|FM|0
11.5|HORMONAL|adrenal_fatigue|Zmęczenie nadnerczy / Adrenal fatigue|AM|0
16.0|HORMONAL|pcos|Zespół policystycznych jajników / PCOS|FM|0
22.0|HORMONAL|menopause|Menopauza / Menopause|AM|0
29.0|HORMONAL|insulin_resistance|Insulinooporność / Insulin resistance|FM|0
37.0|HORMONAL|growth_hormone|Hormon wzrostu / Growth hormone optimization|AM|0
46.0|HORMONAL|testosterone|Optymalizacja testosteronu / Testosterone optimization|FM|0
56.0|HORMONAL|estrogen|Balans estrogenowy / Estrogen balance|AM|0
67.0|HORMONAL|cortisol|Regulacja kortyzolu / Cortisol regulation|FM|0
79.0|HORMONAL|pituitary|Przysadka mózgowa / Pituitary gland|AM|0
92.0|HORMONAL|pineal|Szyszynka / Pineal gland|FM|0
106.0|HORMONAL|parathyroid|Przytarczyce / Parathyroid|AM|0
16.0|HORMONAL|endocrine_balance|Balans hormonalny / Hormonal balance|AM|125
--------------------------------------------------------------------------------
=== DOLEGLIWOŚCI BÓLOWE / PAIN CONDITIONS ===
2.0|PAIN|acute|Ostry ból / Acute pain|AM|0
3.5|PAIN|chronic|Przewlekły ból / Chronic pain|FM|0
5.0|PAIN|back_pain|Ból kręgosłupa / Back pain|AM|0
7.0|PAIN|neck_pain|Ból szyi / Neck pain|FM|0
9.5|PAIN|joint_pain|Ból stawów / Joint pain|AM|0
12.5|PAIN|muscle_pain|Ból mięśni / Muscle pain|FM|0
16.0|PAIN|nerve_pain|Ból nerwowy / Nerve pain|AM|0
20.5|PAIN|headache|Bóle głowy / Headaches|FM|0
26.0|PAIN|fibromyalgia|Fibromialgia / Fibromyalgia|AM|0
33.0|PAIN|sciatica|Rwa kulszowa / Sciatica|FM|0
41.0|PAIN|phantom_limbs|Bóle fantomowe / Phantom limb pain|AM|0
50.0|PAIN|complex_regional|Złożony regionalny zespół bólowy / CRPS|FM|0
60.0|PAIN|post_surgical|Ból pooperacyjny / Post-surgical pain|AM|0
71.0|PAIN|arthritic|Ból artretyczny / Arthritic pain|FM|0
83.0|PAIN|visceral|Ból trzewny / Visceral pain|AM|0
96.0|PAIN|central|Ból centralny / Central pain|FM|0
5.0|PAIN|deep_relief|Głębokie uśmierzanie bólu / Deep pain relief|AM|180
--------------------------------------------------------------------------------
=== INFEKCJE I STANY ZAPALNE / INFECTIONS AND INFLAMMATION ===
3.0|ANTIBACTERIAL|staph|Gronkowiec / Staphylococcus|AM|0
5.0|ANTIBACTERIAL|strep|Paciorkowiec / Streptococcus|FM|0
7.5|ANTIBACTERIAL|ecoli|E. coli|AM|0
10.5|ANTIBACTERIAL|salmonella|Salmonella|FM|0
14.0|ANTIBACTERIAL|pseudomonas|Pseudomonas|AM|0
18.5|ANTIBACTERIAL|klebsiella|Klebsiella|FM|0
24.0|ANTIBACTERIAL|enterococcus|Enterococcus|AM|0
31.0|ANTIBACTERIAL|clostridium|Clostridium|FM|0
39.0|ANTIBACTERIAL|borrelia|Borelioza / Borrelia (Lyme disease)|AM|0
48.0|ANTIBACTERIAL|bartonella|Bartonella|FM|0
58.0|ANTIBACTERIAL|babesia_detailed|Babeszjoza / Babesiosis|AM|0
69.0|ANTIBACTERIAL|ehrlichia|Ehrlichioza / Ehrlichiosis|FM|0
81.0|ANTIBACTERIAL|leptospirosis|Leptospiroza / Leptospirosis|AM|0
94.0|ANTIBACTERIAL|brucellosis|Bruceloza / Brucellosis|FM|0
108.0|ANTIBACTERIAL|anthrax|Wąglik / Anthrax|AM|0
3.0|ANTIVIRAL|influenza|Grypa / Influenza|AM|0
5.5|ANTIVIRAL|common_cold|Przeziębienie / Common cold|FM|0
8.5|ANTIVIRAL|hiv|HIV/AIDS|AM|0
12.0|ANTIVIRAL|hepatitis_b|Wirusowe zapalenie wątroby B / Hepatitis B|FM|0
16.5|ANTIVIRAL|hepatitis_c|Wirusowe zapalenie wątroby C / Hepatitis C|AM|0
22.0|ANTIVIRAL|hpv|HPV (brodawczak ludzki) / Human papillomavirus|FM|0
29.0|ANTIVIRAL|ebv|Epstein-Barr virus|AM|0
37.0|ANTIVIRAL|cmv|Cytomegalowirus / Cytomegalovirus|FM|0
46.0|ANTIVIRAL|hsv|Herpes simplex virus|AM|0
56.0|ANTIVIRAL|vzv|Varicella zoster virus|FM|0
67.0|ANTIVIRAL|enterovirus|Enterowirusy / Enteroviruses|AM|0
79.0|ANTIVIRAL|adenovirus|Adenowirusy / Adenoviruses|FM|0
92.0|ANTIVIRAL|parvovirus|Parwowirus / Parvovirus|AM|0
106.0|ANTIVIRAL|west_nile|West Nile virus|FM|0
121.0|ANTIVIRAL|zika|Zika virus|AM|0
3.5|ANTIFUNGAL|candida|Candida albicans|AM|0
6.0|ANTIFUNGAL|aspergillus|Aspergillus|FM|0
9.0|ANTIFUNGAL|cryptococcus|Cryptococcus|AM|0
13.0|ANTIFUNGAL|histoplasma|Histoplasma|FM|0
18.0|ANTIFUNGAL|coccidioides|Coccidioides|AM|0
24.0|ANTIFUNGAL|blastomyces|Blastomyces|FM|0
31.0|ANTIFUNGAL|pneumocystis|Pneumocystis jirovecii|AM|0
39.0|ANTIFUNGAL|mucor|Mucormycosis|FM|0
48.0|ANTIFUNGAL|sporothrix|Sporothrix|AM|0
58.0|ANTIFUNGAL|dermatophytes|Grzybice skóry / Dermatophytes|FM|0
69.0|ANTIFUNGAL|onychomycosis|Grzybica paznokci / Onychomycosis|AM|0
81.0|ANTIFUNGAL|tinea|Grzybica ciała / Tinea|FM|0
94.0|ANTIFUNGAL|seborrheic|Łojotokowe zapalenie skóry / Seborrheic dermatitis|AM|0
108.0|ANTIFUNGAL|fungal_systemic|Grzybica układowa / Systemic fungal infection|FM|0
7.5|INFLAMMATION|acute|Ostry stan zapalny / Acute inflammation|AM|0
12.0|INFLAMMATION|chronic|Przewlekły stan zapalny / Chronic inflammation|FM|0
18.0|INFLAMMATION|systemic|Stan zapalny całego organizmu / Systemic inflammation|AM|0
25.0|INFLAMMATION|autoimmune_related|Stan zapalny autoimmunologiczny / Autoimmune inflammation|FM|0
33.0|INFLAMMATION|cytokine|Burza cytokinowa / Cytokine storm|AM|0
42.0|INFLAMMATION|crp|Podwyższone CRP / Elevated CRP|FM|0
52.0|INFLAMMATION|esr|Podwyższone OB / Elevated ESR|AM|0
63.0|INFLAMMATION|nf_kappa_b|Inhibicja NF-kappaB / NF-kappaB inhibition|FM|0
75.0|INFLAMMATION|cox2|Inhibicja COX-2 / COX-2 inhibition|AM|0
88.0|INFLAMMATION|tnf_alpha|Redukcja TNF-alfa / TNF-alpha reduction|FM|0
102.0|INFLAMMATION|il6|Redukcja IL-6 / IL-6 reduction|AM|0
117.0|INFLAMMATION|interferon|Stymulacja interferonu / Interferon stimulation|FM|0
12.0|INFLAMMATION|deep_antiinflammatory|Głębokie działanie przeciwzapalne / Deep anti-inflammatory|AM|145
--------------------------------------------------------------------------------
=== ZABURZENIA PSYCHICZNE I EMOCJONALNE / MENTAL AND EMOTIONAL DISORDERS ===
2.5|MENTAL|anxiety|Lęk i niepokój / Anxiety|AM|0
4.0|MENTAL|depression|Depresja / Depression|FM|0
6.0|MENTAL|panic_attacks|Ataki paniki / Panic attacks|AM|0
8.5|MENTAL|ptsd|PTSD / Post-traumatic stress disorder|FM|0
11.5|MENTAL|ocd|OCD / Obsessive-compulsive disorder|AM|0
15.0|MENTAL|bipolar|Choroba afektywna dwubiegunowa / Bipolar disorder|FM|0
19.5|MENTAL|schizophrenia|Schizofrenia / Schizophrenia|AM|0
25.0|MENTAL|adhd|ADHD / Attention deficit hyperactivity disorder|FM|0
31.5|MENTAL|autism|Spektrum autyzmu / Autism spectrum|AM|0
39.0|MENTAL|eating_disorder|Zaburzenia odżywiania / Eating disorders|FM|0
47.5|MENTAL|addiction|Uzależnienia / Addictions|AM|0
57.0|MENTAL|insomnia|Bezsenność / Insomnia|FM|0
67.5|MENTAL|stress|Stres przewlekły / Chronic stress|AM|0
79.0|MENTAL|burnout|Wypalenie zawodowe / Burnout syndrome|FM|0
91.5|MENTAL|mood_swings|Wahania nastroju / Mood swings|AM|0
105.0|MENTAL|emotional_trauma|Trauma emocjonalna / Emotional trauma|FM|0
120.0|MENTAL|grief|Żałoba / Grief processing|AM|0
136.0|MENTAL|anger_management|Kontrola gniewu / Anger management|FM|0
6.0|MENTAL|calm|Głęboki spokój / Deep calm|AM|95
11.5|MENTAL|focus|Koncentracja i skupienie / Focus and concentration|FM|80
--------------------------------------------------------------------------------
=== DETOKSYKACJA / DETOXIFICATION ===
8.0|DETOX|heavy_metals|Metale ciężkie / Heavy metals|AM|0
13.0|DETOX|mercury|RTĘĆ / Mercury|FM|0
19.0|DETOX|lead|OŁÓW / Lead|AM|0
26.0|DETOX|cadmium|KADM / Cadmium|FM|0
34.0|DETOX|aluminum|GLIN / Aluminum|AM|0
43.0|DETOX|arsenic|ARSEN / Arsenic|FM|0
53.0|DETOX|chemicals|Toksyny chemiczne / Chemical toxins|AM|0
64.0|DETOX|pesticides|Pestycydy / Pesticides|FM|0
76.0|DETOX|herbicides|Herbicydy / Herbicides|AM|0
89.0|DETOX|industrial|Toksyny przemysłowe / Industrial toxins|FM|0
103.0|DETOX|radiation|Promieniowanie / Radiation detox|AM|0
118.0|DETOX|chemotherapy|Detoks po chemioterapii / Post-chemotherapy detox|FM|0
134.0|DETOX|drug_detox|Detoks lekowy / Drug detoxification|AM|0
151.0|DETOX|alcohol|Detoks alkoholowy / Alcohol detox|FM|0
169.0|DETOX|nicotine|Detoks nikotynowy / Nicotine detox|AM|0
188.0|DETOX|environmental|Toksyny środowiskowe / Environmental toxins|FM|0
13.0|DETOX|liver_support|Wsparcie wątroby / Liver support|AM|115
26.0|DETOX|kidney_support|Wsparcie nerek / Kidney support|FM|100
--------------------------------------------------------------------------------
=== OCZY I USZY / EYES AND EARS ===
3.5|SENSORY|cataracts|Zaćma / Cataracts|AM|0
5.5|SENSORY|glaucoma|Jaskra / Glaucoma|FM|0
8.0|SENSORY|macular_degeneration|Zwyrodnienie plamki żółtej / Macular degeneration|AM|0
11.0|SENSORY|retinitis|Zapalenie siatkówki / Retinitis|FM|0
14.5|SENSORY|conjunctivitis|Zapalenie spojówek / Conjunctivitis|AM|0
18.5|SENSORY|dry_eyes|Zespół suchego oka / Dry eye syndrome|FM|0
23.0|SENSORY|eye_strain|Zmęczenie oczu / Eye strain|AM|0
28.0|SENSORY|night_blindness|Ślepota nocna / Night blindness|FM|0
33.5|SENSORY|color_blindness|Daltonizm / Color blindness|AM|0
39.5|SENSORY|floaters|Męty w oku / Eye floaters|FM|0
46.0|SENSORY|hearing_loss|Utrata słuchu / Hearing loss|AM|0
53.0|SENSORY|tinnitus|Szumy uszne / Tinnitus|FM|0
60.5|SENSORY|ear_infection|Infekcja ucha / Ear infection|AM|0
68.5|SENSORY|vertigo_detailed|Zawroty głowy - ucho wewnętrzne / Inner ear vertigo|FM|0
77.0|SENSORY|otosclerosis|Otoskleroza / Otosclerosis|AM|0
86.0|SENSORY|meniere|Choroba Meniere'a / Meniere's disease|FM|0
95.5|SENSORY|auditory_nerve|Nerw słuchowy / Auditory nerve|AM|0
105.5|SENSORY|optic_nerve|Nerw wzrokowy / Optic nerve|FM|0
8.0|SENSORY|vision_improve|Poprawa wzroku / Vision improvement|AM|75
53.0|SENSORY|hearing_improve|Poprawa słuchu / Hearing improvement|FM|70
--------------------------------------------------------------------------------
FREQUENCY_DATA_END

PROTOCOL_GUIDELINES:
- Acute session: 3-10 minutes per frequency
- Chronic session: 10-30 minutes per frequency
- Acute cycle: 2-3 times daily for 3-7 days
- Chronic cycle: 1-2 times daily for 2-8 weeks
- Maintenance: 2-3 times per week

SAFETY_WARNINGS:
- Pregnancy: Consult healthcare provider before use
- Pacemaker: Contraindicated - do not use
- Metal implants: Use with caution, lower intensity
- Seizure disorders: Avoid certain frequencies without medical supervision
- Cancer: Consult oncologist before use
- Children: Use reduced intensity and duration

DISCLAIMER: These frequencies are based on research in frequency medicine including works by Royal Rife, Hulda Clark, Dr. Joseph Mercola, Dr. William Tiller, and other bioresonance pioneers. Individual protocols should be customized based on specific case, health status, and patient response. Always consult with a qualified bioresonance practitioner before applying any frequency therapy. Some frequencies may interact with pacemakers, metal implants, pregnancy, and other medical conditions.
-->

# Wstęp do biorezonansu

Biorezonans to metoda terapeutyczna wykorzystująca fale elektromagnetyczne o określonych częstotliwościach do diagnostyki i leczenia różnych schorzeń. Działa na zasadzie rezonansu - każda komórka, tkanka i organ ma swoją charakterystyczną częstotliwość, a odpowiednie dobranie częstotliwości może wspomóc procesy samoleczenia organizmu.

## Zasada działania

Terapia biorezonansem polega na aplikowaniu do organizmu fal elektromagnetycznych o precyzyjnie dobranych częstotliwościach, które:
- Wprowadzają chore komórki w stan rezonansu
- Przywracają prawidłowe wibracje komórkowe
- Stymulują naturalne procesy naprawcze
- Wspierają detoksykację organizmu

## Rodzaje modulacji

1. **AM (Amplitude Modulation)** - modulacja amplitudy, skuteczna dla tkanek miękkich
2. **FM (Frequency Modulation)** - modulacja częstotliwości, penetruje tkanki twarde
3. **PWM (Pulse Width Modulation)** - modulacja szerokości impulsu, idealna dla układu nerwowego
4. **BURST** - serie impulsów dla maksymalnej absorpcji energii
5. **SWEEP** - przemiatanie częstotliwości dla uniknięcia adaptacji tkanek
6. **RANDOM** - losowe zmiany dla zapobiegania tolerancji

## Format danych

Częstotliwości w komentarzu HTML powyżej są sformatowane w sposób czytelny zarówno dla człowieka, jak i oprogramowania:
- **Kolumny oddzielone pipe (`|`)**: `częstotliwość_hz|kategoria|podkategoria|opis|modulacja|nośna_khz`
- **Łatwe parsowanie**: Można wyodrębnić za pomocą prostych funkcji split() w dowolnym języku programowania
- **Kategorie ustandaryzowane**: Ułatwiają filtrowanie i wyszukiwanie
- **Opisy dwujęzyczne**: Polski i angielski dla międzynarodowego użycia

## Bezpieczeństwo

Przed rozpoczęciem terapii biorezonansem należy skonsultować się z lekarzem, szczególnie w przypadkach:
- Ciąży
- Posiadania rozrusznika serca
- Obecności implantów metalowych
- Ciężkich schorzeń przewlekłych
- Chorób neurologicznych (padaczka)

---

*Szczegółowa baza danych częstotliwości znajduje się w formacie machine-readable w komentarzu HTML na początku tego pliku.*

# Wstęp do biorezonansu

Biorezonans to metoda terapeutyczna wykorzystująca fale elektromagnetyczne o określonych częstotliwościach do diagnostyki i leczenia różnych schorzeń. Działa na zasadzie rezonansu - każda komórka, tkanka i organ ma swoją charakterystyczną częstotliwość, a odpowiednie dobranie częstotliwości może wspomóc procesy samoleczenia organizmu.

## Zasada działania

Terapia biorezonansem polega na aplikowaniu do organizmu fal elektromagnetycznych o precyzyjnie dobranych częstotliwościach, które:
- Wprowadzają chore komórki w stan rezonansu
- Przywracają prawidłowe wibracje komórkowe
- Stymulują naturalne procesy naprawcze
- Wspierają detoksykację organizmu

## Rodzaje modulacji

1. **AM (Amplitude Modulation)** - modulacja amplitudy, skuteczna dla tkanek miękkich
2. **FM (Frequency Modulation)** - modulacja częstotliwości, penetruje tkanki twarde
3. **PWM (Pulse Width Modulation)** - modulacja szerokości impulsu, idealna dla układu nerwowego

## Bezpieczeństwo

Przed rozpoczęciem terapii biorezonansem należy skonsultować się z lekarzem, szczególnie w przypadkach:
- Ciąży
- Posiadania rozrusznika serca
- Obecności implantów metalowych
- Ciężkich schorzeń przewlekłych

---

*Szczegółowa lista częstotliwości znajduje się w komentarzu HTML na początku tego pliku.*
