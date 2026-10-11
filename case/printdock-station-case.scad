// PrintDock Station – Gehäuse zum Drucken (OpenSCAD)
// ---------------------------------------------------------------------------
// Teile:   part = "base"  Unterteil mit Halterungen für Board, HX711, OLED, LED, Taster
//          part = "deck"  Deckel mit Schienen für den PN532 (wird eingeklickt)
//          part = "plate" Teller mit Pfosten (liegt auf der Wägezelle)
//          part = "all"   alles zusammen (nur zum Ansehen)
// Exportieren: openscad -D 'part="base"' -o base.stl printdock-station-case.scad
//
// Zusammenbau – nur 4 Schrauben (die der Wägezelle), alles andere wird gesteckt bzw. geklickt:
//   1. Wägezelle mit 2 Schrauben von unten an den Sockel (festes Ende, Pfeil auf der Zelle zeigt nach unten).
//   2. HX711 in die Mulde, ESP32/D1 Mini in die Bucht (Doppelklebeband), USB nach hinten.
//   3. OLED, Taster-Modul und LED von oben in ihre Schlitze an der Front schieben.
//   4. PN532 in die Schienen unter dem Deckel schieben, Deckel einklicken.
//   5. Teller mit 2 Schrauben von oben durch den Pfosten an das freie Ende der Zelle.
//
// Druck: PLA/PETG, 0,2 mm, 3 Wände, 15–20 % Füllung, ohne Stützen.
//   base:  wie sie liegt (Boden unten)      deck: Oberseite nach unten      plate: Tellerfläche nach unten
// ---------------------------------------------------------------------------

part = "all";

/* [Gehäuse] */
W = 150;          // Breite (x)
D = 150;          // Tiefe (y)
H = 38;           // Höhe Unterteil (außen, ohne Deckel)
wall = 2.4;
floor_t = 2.4;
r_corner = 12;
lid_t = 2.4;      // Deckelstärke
gap = 0.3;        // Spiel für Steck- und Klickverbindungen

/* [Wägezelle 5 kg, 80 x 12,7 x 12,7] */
cell_l = 80;
cell_w = 12.7;
cell_h = 12.7;
cell_y = 34;            // Lage (Mitte) in y
cell_x0 = -55;          // festes Ende (links)
cell_hole_d = 5.6;      // passt für M4 und M5
cell_hole_end = 5;      // Abstand erstes Loch vom Ende
cell_hole_pitch = 15;   // Lochabstand
cell_boss_h = 5;        // Sockel unter dem festen Ende – die Zelle schwebt sonst frei

/* [PN532 V3, unter dem Deckel] */
pn_l = 43.2;      // Schieberichtung (x)
pn_w = 40.9;      // zwischen den Schienen (y)
pn_t = 1.7;       // Platinenstärke
pn_drop = 0.8;    // Abstand Platine – Deckel
pn_window = 1.2;  // Deckel über dem Leser nur so dick (Reichweite)

/* [OLED 0,96", Taster KY-004, WS2812-Rund] */
oled_w = 27.6; oled_h = 27.9; oled_t = 1.7;
oled_win_w = 24; oled_win_h = 13; oled_win_z = 19;   // Fenstermitte über Boden
btn_w = 18.8; btn_t = 1.7; btn_depth = 7.0;           // Platinenvorderseite hinter der Wand
btn_switch_h = 5.0;                                   // Höhe Tastknopf über der Platine
oled_glass = 2.0;                                     // Luft für das Glas vor der OLED-Platine
hook_off = 50;                                        // Lage der Klickhaken (aus der Seitenmitte gerückt)
led_d = 10.4; led_t = 3.2;
front_mid_z = 20;

/* [Board-Bucht] */
bay_x0 = 37; bay_x1 = W/2 - wall - 0.5; bay_y0 = 14;
usb_w = 16; usb_h = 22;

/* [Teller] */
plate_d = 140;
plate_t = 3;
rim_h = 0;        // kein Rand – so druckt der Teller flach ohne Stützen
plate_gap = 2.2;  // Luft zwischen Deckel und Teller

$fn = 64;
eps = 0.01;

// ---------------------------------------------------------------- abgeleitete Maße
cell_z0 = floor_t + cell_boss_h;          // Unterkante Zelle
cell_z1 = cell_z0 + cell_h;               // Oberkante Zelle
lid_z = H;                                // Deckel liegt auf den Wänden
plate_z = H + lid_t + plate_gap;          // Unterseite Teller
post_x0 = cell_x0 + cell_l - 25;          // Pfosten nur auf dem freien Ende (die Mitte der Zelle muss frei biegen)
post_x1 = cell_x0 + cell_l + 1;
post_w = 15;
free_holes = [cell_x0 + cell_l - cell_hole_end, cell_x0 + cell_l - cell_hole_end - cell_hole_pitch];
fixed_holes = [cell_x0 + cell_hole_end, cell_x0 + cell_hole_end + cell_hole_pitch];

module rbox(w, d, h, r) {
  translate([-w/2 + r, -d/2 + r, 0]) minkowski() { cube([w - 2*r, d - 2*r, h/2]); cylinder(r = r, h = h/2); }
}

// ---------------------------------------------------------------- Unterteil
module base() {
  difference() {
    union() {
      difference() {
        rbox(W, D, H, r_corner);
        translate([0, 0, floor_t]) rbox(W - 2*wall, D - 2*wall, H, r_corner - wall);
      }
      // Sockel für das feste Ende der Wägezelle
      translate([cell_x0 - 2, cell_y - cell_w/2 - 3, 0]) cube([cell_hole_end + cell_hole_pitch + 9, cell_w + 6, cell_z0]);
      // OLED: zwei senkrechte Nuten hinter der Front
      for (s = [-1, 1]) translate([s * (oled_w/2), -D/2 + wall + oled_glass, 0]) slot_post(s, oled_t + 0.4, H - 4);
      // Taster-Modul (KY-004): Nuten, Platine steht btn_depth hinter der Wand
      translate([30, 0, 0]) for (s = [-1, 1]) translate([s * (btn_w/2), -D/2 + wall + btn_depth, 0]) slot_post(s, btn_t + 0.4, H - 4);
      // LED: runde Platine in einer Tasche hinter der Wand
      translate([-30, -D/2 + wall, 0]) led_holder();
      // HX711-Mulde (Rahmen)
      translate([-W/2 + wall + 4, -24, floor_t]) difference() { cube([35.6 + 3, 21.6 + 3, 3]); translate([1.5, 1.5, -eps]) cube([35.6, 21.6, 4]); }
      // Rahmen der Board-Bucht (Lage fürs Klebeband)
      translate([bay_x0, bay_y0, floor_t]) difference() { cube([bay_x1 - bay_x0, D/2 - wall - bay_y0, 1.6]); translate([1.2, 1.2, -eps]) cube([bay_x1 - bay_x0 - 2.4, D, 2]); }
    }
    // Schraublöcher festes Ende (von unten, Senkung für den Kopf)
    for (x = fixed_holes) translate([x, cell_y, -eps]) { cylinder(d = cell_hole_d, h = cell_z0 + 1); cylinder(d = 10, h = 2.2); }
    // OLED-Fenster
    translate([0, -D/2 - eps, oled_win_z]) cube([oled_win_w, wall * 3, oled_win_h], center = true);
    // LED-Lichtloch
    translate([-30, -D/2 - eps, front_mid_z]) rotate([-90, 0, 0]) cylinder(d = 5, h = wall + 1);
    // Taster: federnde Zunge in der Front (U-Schlitz), oben angebunden
    translate([30, -D/2 + wall/2, front_mid_z]) tongue_cut();
    // USB-Öffnung hinten in der Board-Bucht
    translate([(bay_x0 + bay_x1)/2, D/2 - wall/2, floor_t + usb_h/2]) cube([usb_w, wall * 3, usb_h], center = true);
    // Kabeldurchlass Zelle → HX711 (falls Kabel unter dem Sockel)
    // Fenster für die Klickhaken des Deckels
    for (a = [0, 90, 180, 270]) rotate([0, 0, a]) translate([-hook_off, D/2 - wall/2, H - 6]) cube([10.6, wall * 3, 2.8], center = true);
    // Mulden für Gummifüße
    for (sx = [-1, 1], sy = [-1, 1]) translate([sx * (W/2 - 16), sy * (D/2 - 16), -eps]) cylinder(d = 11, h = 1);
  }
  // Druckstift der Zunge (innen)
  translate([30, -D/2 + wall - eps, front_mid_z]) rotate([-90, 0, 0]) cylinder(d1 = 4, d2 = 3, h = btn_depth - btn_switch_h - 0.3);
}

// senkrechter Steg mit Nut (Platine wird von oben eingeschoben)
// senkrechter Steg mit Nut zur Platine hin – Platine wird von oben eingeschoben.
// Ursprung: Platinenkante (x) und Platinenvorderseite (y); side = +1 rechte Kante, -1 linke Kante.
module slot_post(side, slot, h) {
  depth = 1.6;            // so weit greift die Nut über die Platinenkante
  bx = side > 0 ? -depth : -3.2;
  difference() {
    translate([bx, -1.6, 0]) cube([depth + 3.2, slot + 3.2, h]);
    translate([side > 0 ? -depth - eps : -0.3, 0, floor_t + 1]) cube([depth + 0.3 + eps, slot, h]);
  }
}

module led_holder() {
  // Tasche: LED-Platine (rund 10 mm) liegt flach an der Wand, von oben eingeschoben
  difference() {
    translate([-led_d/2 - 2, 0, floor_t]) cube([led_d + 4, led_t + 2, front_mid_z + led_d/2 + 2 - floor_t]);
    translate([-led_d/2, -eps, front_mid_z - led_d/2]) cube([led_d, led_t + 0.3, H]);
    translate([-led_d/2 + 2, -eps, front_mid_z - led_d/2 + 2]) cube([led_d - 4, led_t + 3, H]);
  }
}

module tongue_cut() {
  tw = 12; th = 14; s = 0.9;
  difference() {
    cube([tw + 2*s, wall * 3, th + s], center = true);
    translate([0, 0, s]) cube([tw, wall * 4, th + s], center = true);
  }
}

// ---------------------------------------------------------------- Deckel
module deck() {
  inner_w = W - 2*wall - 2*gap;
  inner_d = D - 2*wall - 2*gap;
  difference() {
    union() {
      translate([0, 0, lid_z]) rbox(W, D, lid_t, r_corner);
      // Zentrierrand innen
      translate([0, 0, lid_z - 3]) difference() {
        rbox(inner_w, inner_d, 3 + eps, r_corner - wall);
        translate([0, 0, -eps]) rbox(inner_w - 3.2, inner_d - 3.2, 4, r_corner - wall - 1.6);
      }
      // Klickhaken
      for (a = [0, 90, 180, 270]) rotate([0, 0, a]) translate([-hook_off, inner_d/2 - 0.8, lid_z]) hook();
      // Schienen für den PN532 (Platine von der Seite einschieben, Bauteile nach unten)
      for (s = [-1, 1]) rail(s);
      // Anschlag
      translate([pn_l/2 + 0.3, -pn_w/2, lid_z - pn_drop - pn_t - 0.4]) cube([2, pn_w, pn_drop + pn_t + 0.4]);
    }
    // dünnes Fenster über dem Leser (Reichweite) – von unten ausgespart, Oberseite bleibt glatt
    translate([-pn_l/2 + 2, -pn_w/2 + 2, lid_z - eps]) cube([pn_l - 4, pn_w - 4, lid_t - pn_window + eps]);
    // Durchgang für den Tellerpfosten
    translate([post_x0 - 3.5, cell_y - post_w/2 - 3.5, lid_z - 5]) cube([post_x1 - post_x0 + 7, post_w + 7, lid_t + 10]);
    // Schlitze neben den Haken (federn)
  }
}

module hook() {
  // Zunge nach unten mit Nase nach außen, rastet in das Fenster der Wand
  tw = 9.6;
  translate([-tw/2, -1.4, -9]) cube([tw, 1.4, 9 + eps]);
  // Nase: oben gerade (rastet), unten schräg (gleitet beim Aufdrücken)
  translate([-tw/2, -eps, -9 + 1.6]) hull() { cube([tw, eps, 2.4]); translate([0, 1.9, 1.4]) cube([tw, eps, 1.0]); }
}

module rail(s) {
  // Schiene unter dem Deckel: Nut nimmt die Platinenkante auf (1,5 mm), Platine von -x einschieben
  zb = lid_z - pn_drop - pn_t;            // Unterkante Platine
  y_in = pn_w/2 - 1.5; y_out = pn_w/2 + 2.2;
  difference() {
    translate([-pn_l/2 - 1, s > 0 ? y_in : -y_out, zb - 1.6]) cube([pn_l + 1, y_out - y_in, lid_z - zb + 1.6 + eps]);
    translate([-pn_l/2 - 2, s > 0 ? y_in - eps : -(pn_w/2 + 0.3), zb - 0.1]) cube([pn_l + 4, 1.5 + 0.3 + eps, pn_t + 0.25]);
  }
}

// ---------------------------------------------------------------- Teller
module plate() {
  difference() {
    union() {
      translate([0, 0, plate_z]) cylinder(d = plate_d, h = plate_t);
      // Pfosten bis auf das freie Ende der Zelle
      translate([post_x0, cell_y - post_w/2, cell_z1]) cube([post_x1 - post_x0, post_w, plate_z - cell_z1 + eps]);
    }
    for (x = free_holes) translate([x, cell_y, cell_z1 - 1]) {
      cylinder(d = cell_hole_d, h = 60);
      translate([0, 0, plate_z + plate_t - cell_z1 + 1 - 3.5]) cylinder(d = 10, h = 10); // Kopfsenkung von oben
    }
    // Gewicht sparen
    for (a = [30:60:330]) rotate([0, 0, a]) translate([42, 0, plate_z - 1]) cylinder(d = 26, h = plate_t - 1.4 + 1);
  }
}

// Wägezelle nur zum Ansehen
module cell() {
  color("silver") difference() {
    translate([cell_x0, cell_y - cell_w/2, cell_z0]) cube([cell_l, cell_w, cell_h]);
    translate([cell_x0 + cell_l/2, cell_y, cell_z0 + cell_h/2]) rotate([90, 0, 0]) hull() { for (x = [-8, 8]) translate([x, 0, 0]) cylinder(d = 8, h = cell_w + 2, center = true); }
  }
}

if (part == "base") base();
else if (part == "deck") deck();
else if (part == "plate") plate();
else {
  color("#3F5BD8") base();
  cell();
  color("#9AA7B8", 0.8) translate([0, 0, 18]) deck();
  color("#F2F2F2") translate([0, 0, 40]) plate();
}
