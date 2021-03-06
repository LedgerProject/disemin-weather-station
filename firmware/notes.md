## DataStore
Για να αποφευχθούν οι περίσσιες εγγραφές και το wear της flash, δημιουργήθηκε το DataStore class. Κρατάει buffer των δεδομένων στη RAM και όταν γεμίσει ο buffer αυτόματα (η χειροκίνητα οταν κληθεί commit()) τα γράφει στη flash δημιουργώντας 1 αρχείο για κάθε commit. Υπάρχει αντίστοιχα και το DataStoreReader ο οποιος είναι iterator για τα δεδομένα σε ενα DataStore, και επιστρέφει μια μια τις εγγραφές ξεκινώντας από τη RAM και συνεχίζοντας στη flash μέχρι να επιστραφούν όλες, τελείως transparent για τον χρήστη της κλάσης.
Με αυτόν τον τρόπο η μνήμη flash δεν χρησιμοποιείται σχεδόν καθόλου.
Προβλήματα μπορούν να προκύψουν σε περίπτωση που υπάρχει διακοπή ρεύματος ή κάποιο σφάλμα για το οποίο θα γίνει reset. Τα δεδομένα που δεν έχουν γίνει commit χάνονται πράγμα μπορεί να δυσκολέψει το debugging στην περίπτωση του log DataStore.
Πιθανή λύση θα ήταν να γίνεται εγγραφή στη flash κάθε φορά που προστίθεται κάποια καταχώρηση, χωρίς τη χρήση buffer. Τότε έχουμε 2 περιπτώσεις:

1. Γράφουμε ένα αρχείο για κάθε entry στην οποία περίπτωση σπαταλάμε πολύ χώρο γιατί το page size είναι 512 οπότε χάνονται τα περισσότερα. Και κάθε φορά που στέλνουμε δεδομένα, όταν η αποστολή για ένα αρχείο είναι επιτυχής, το σβήνουμε, εαν όχι μένει και θα σταλθεί την επόμενη φορά.
Επίσης ο χρόνος δημιουργίας καινούριου αρχείου είναι πολύ μεγάλος στο SPIFFS (~300ms) αλλα αυτό δεν είναι πρόβλημα για τη μπαταρία γιατί έχουμε μόλις μερικά αρχεία την ώρα.
2. Γράφουμε ένα μεγάλο αρχείο στο οποίο κάνουμε append κάθε φορά τα καινούρια entries. Όταν έρχεται η ώρα της αποστολής, όσα αποσταλούν μαρκάρονται ως απεσταλμένα, τα υπόλοιπα όχι για να ξαναγίνει προσπάθεια την επόμενη φορά. Εαν σταλούν όλα, τότε το αρχείο σβήνεται. Σε αυτή τη περίπτωση το overhead των metadata μοιράζεται, αλλά υπάρχει περίπτωση εαν κάθε φορά που γίνεται αποστολή υπάρχει ενα entry που αποτύχει, τότε το αρχείο δεν θα σβηστεί ποτέ και θα μεγαλώνει συνέχεια μέχρι να γεμίσει ο χώρος. Τότε κανονικά θα πρέπει να γίνει εκκαθάριση χώρου με το να σβηστούν τα Χ παλαιότερα αρχεία αλλά υπάρχει μόνο ένα αρχείο το οποίο έχει και τα τελευταία δεδομένα που δεν εστάλησαν και αναγκαστικά θα σβηστεί για να κάνει χώρο για νέα δεδομένα οπότε και έχουμε απώλεια δεδομένων.

H μέση λύση που σπαταλάει ελάχιστο χώρο σε metadata και ελαχιστοποιεί το τελευταίο πρόβλημα σε σημείο να είναι απίθανο να συναντηθεί:
Δεν δημιουργείται καινούριο αρχείο για κάθε καταχώρηση, αλλά γράφονται Χ bytes σε ένα αρχείο μέχρι να δημιουργηθεί καινούριο. Έτσι εαν στη χείριστη περίπτωση να έχουμε πχ. 20 αρχεία με 1 εγγραφή που απέτυχε να σταλεί στο καθένα στο καθένα, και γεμίσει ο χώρος, διαγράφεται το παλαιότερο από τα αρχεία. Αυτή η διαδικασία γίνεται αυτόματα στο commit.


## NTP wrong Timezones
Ορίζοντας ως pool.ntp.org για NTP server στο GSM module, έπαιρνα για κάποιο λογο local time (UTC+03) άλλα στο string που επέστρεφε δεν υπήρχε αυτή η πληροφορία πχ:
05/04/21,22:06:15+00

Αλλάζοντας το ΝΤΡ server σε 1.pool.ntp.org άρχισε να επιστρέφει σωστή ώρα. Γυρνώντας το πάλι σε pool.ntp.org οι ώρες πάλι επέστρεφαν σωστά οπότε δεν μπόρεσα να το αναπαράξω. Δοκιμάζοντας επόμενες φορές επέστρεφε πάντα local χρόνο (δηλαδή λάθος) και φαίνεται να είναι τυχαίο.
Η ΑΤ εντολή είναι AT+CLTS, δηλαδή Local Timestamp οπότε ίσως αυτό εξηγεί γιατί πάντα επιστρέφει local. Γιατί όμως το timezone κομμάτι του time string είναι 0 και γιατί κάποιες ελάχιστες φορές το επέστρεψε σωστά;

update 1: Απ'οτι φαίνεται εαν ζητήσεις να κάνει ΝΤΡ sync αλλά δεν έχει ανοίξει το GPRS (ή αποτύχει το req?) σου δίνει την ώρα του δικτύου η οποία έχει τα παραπάνω συμπτώματα. Αν πρώτα ανοίξεις το GPRS κάνει το σωστό sync.

## GSM Modules
Έχουμε 2 modules:
1. Πράσινο (ebay)
    Παίρνει από 5-10V
2. RPi hat (waveshare)
    Παίρνει 5v

Και στα 2 modules μερικές φορές σε τυχαίες στιγμές ενω γίνεται η επικοινωνία με το module, μερικές φορές μπορεί να μην έρχεται απάντηση η γενικά κάτι να αποτυγχάνει.
Σε αυτές τις περιπτώσεις ανεβάζοντας την τάση στα 5.5V διορθώνεται. Στα 5v δεν υπάρχει πάντα πρόβλημα,κάποιες φορές λειτουργεί κανονικά απλά δεν είναι αξιόπιστο. Το SIM7000 περιμένει power input κατευθείαν από λιθίου. Για αυτή τη δουλειά το (1) έχει LDO και το (2) έχει step down.
Το hat είναι αρκετα πιο πολύπλοκο από το πράσινο οπότε έβγαλα το LDO από το πράσινο και έδωσα ρεύμα απευθείας από τη μπαταρία (αφού το SIM7000 έχει προβλευθεί για αυτό).

Δοκιμάζοντας λειτουργία από μπαταρίες:
Hat:
1. Έκανε συνέχεια brownout.

Πράσινο
1. 1 καινούρια μπαταρία είχε θέματα, και μερικές φορές brownout.
2. 2 καινουριες μπαταρίες φαίνεται να λειτουργούσε εντάξει
3. 1 παλιά/προβληματική μπαταρία συχνά brown out
4. 2 παλιές/προβληματικές μπαταρίες δεν συνδεόταν

Εβαλα και ενα 1000uF bypass cap και τώρα φαίνεται να λειτουργεί καλύτερα.

---

Δοκιμάζοντας το μπλέ module σε 6v πέτυχα την καλύτερη σταθερότητα μέχρι τώρα με επιτυχία ~80%, εκτός απο μερικές φορές που πεισμώνει και αρνείται να συνδεθεί.

---

Το GSM module άρχισε να βγάζει κατεστραμμένους χαρακτήρες ("????") μερικές φορές, την ώρα που κάνει connect/transmit. Δοκιμάστηκαν διάφορα GPIO με παρόμοια η χειρότερα αποτελέσματα.
Δοκιμές που έγιναν με Adafruit Huzzah, DFRobot Firebeetle και NodeMCU ESP32 είχαν πολύ καλύτερα αποτελέσματα με τους κατεστραμμένους χαρακτήρες να εμφανίζονται πολύ λιγότερο. Αλλάζοντας την κάρτα SIM σε Q οι κατεστραμμένοι χαρακτήρες μειώθηκαν δραματικά και άρχισαν να εμφανίζονται που και που 1-2 και ταυτόχρονα τα requests άρχισαν να πετυχαίνουν σε μεγαλύτερα ποσοστά.
Με την αλλαγή των TX/RX pins από 16/17 σε 22/17 τα πράγματα βελτιώθηκαν λίγο ακόμα, όμως άρχισε πάλι να χαλάει το RTC και επέστρεφε μη έγκυρα timestamps.



## RTC
Έχουμε 3 RTC στο data logger:
1. Εσωτερικό RTC του GSM module: Δεν χρησιμοποιείται γιατί δεν είναι υποστηριζόμενο απο εξωτερική μπαταρία οπότε χάνει την ώρα κάθε φορά που του κόβουμε την παροχή.
2. Εξωτερικό RTC του ΤFox (DS3231): Υποστηρίζεται απο εξωτερική μπαταρία και χρησιμοποιείται ως η πηγή συχρονισμού του εσωτερικού RTC του ESP32 όταν δεν είναι δυνατός ο συγχρονισμός με NTP.
3. Εσωτερικό RTC του ESP32: Το βασικό ρολόι του συστήματος. Δεν υποστηρίζεται από εξωτερική μπαταρία οπότε συγχρονίζεται σε κάθε εκκίνηση με NTP ή εξωτερικό RTC (όποιο είναι διαθεσιμο). Είναι η βασική πηγή ώρας επειδή το ESP-idf υλοποιεί στάνταρ βιβλιοθήκες ώρας της C, οπότε οποιοαδήποτε βιβλιοθήκη χρησιμοποιεί την ώρα θα λειτουργεί. Οι άλλες 2 πηγες (GSM/NTP και εξωτερικό RTC) χρησιμοποιούνται για να μπορούν να συγχρονίζουν αυτό το RTC.

Η διαδικασία συγχρονισμού του ρολογιού γίνεται σε κάθε εκκίνηση και έχει ως εξής:
1. Απόπειρα συγχρονισμού του εσωτερικού RTC που έχει το GSM module μέσω NTP.
    Αν πετύχει αυτό ενημερώνονται το εξωτερικό RTC και το RTC του ESP32.
2. Αν αποτύχει το NTP, το εσωτερικό RTC του ESP32 ενημερώνεται από το εξωτερικό RTC.


Σε τυχαίες στιγμές το εξωτερικό RTC επιστρέφει ώρα στο μέλλον (2025+) και μερικές φορές στο παρελθόν (2000) ενω τις περισσότερες φορές δείχνει να λειτουργεί κανονικά. Κάποιες φορές αποτυγχάνει με το μήνυμα:
"[E][esp32-hal-i2c.c:1426] i2cCheckLineState(): Bus Invalid State, TwoWire() Can't init sda=0, scl=1
Could not begin I2C comms with ext RTC."
Κάνοντας init το εξωτερικό RTC κάθε φορά πριν του ζητήσουμε την ώρα φαίνεται να λύνει το πρόβλημα, αλλά με συχνότερο init αυξάνεται η συχνότητα του παραπάνω μηνύματος.

--
Φαίνεται να επηρεάζεται για κάποιο λόγο από το GSM. Αφού έτρεχε το NTP sync μέσω GSM, ενημερωνε το RTC για την ώρα. Εκεί χαλούσε καποιες φορές. Αλλάζοντας τα pins του GSM (το ενα απο τα οποία ήταν κοντά σε ενα απο τα i2c channels) έφυγε το πρόβλημα, ίσως κάποια παρεμβολή  χαλούσε την κυματομορφή.

## Diagnostic telemetry
Κατα την αποστολή των logs, κάποια από τα log codes επιλέγονται για αποστολη ως telemetry στο TB για να βοηθήσουν στα διαγνωστικά:
- d_gsm_conn_fail
Απέτυχε να συνδεθεί στο δίκτυο τελείως (χάλια σήμα)

- d_gsm_rssi
RSSI

- d_tm_awake_s
Καταγράφεται αμέσως πριν κοιμηθεί (οπότε είναι και sleep event) και η τιμή είναι ο χρόνος που έιναι ξύπνιο (seconds).
Είναι μηδέν προφανώς στο πρώτο sleep.

- d_wakeup
Wake up event

- d_sd_total_rec
Σύνλογο εγγραφών Sensor Data (sd) που εστάλησαν σε αυτη την αποστολή.

- d_sd_crc
Πόσες από τις εγγραφές απέτυχαν λόγω CRC error σε αυτη την αποστολή.

- d_sd_total_req
Σύνολο requests που έγιναν σε αυτη την αποστολή.

- d_sd_failed_req
Πόσα από τα requests απέτυχαν  σε αυτη την αποστολή.