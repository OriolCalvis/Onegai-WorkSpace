"""Materiales de los personajes: rampas de cinco tonos (luz, base, sombra,
contorno suave, contorno duro) para pieles, pelo, telas, cuero y metal."""
from actores_pintura import Material as M

MATERIALES = {m.nombre: m for m in [
    # telas
    M("tunica_verde", "#8cbf5a", "#5a9442", "#3a6a33", "#1f3b1c", "#142814"),
    M("tunica_azul", "#78a6e0", "#4a74b8", "#314f86", "#1b2c4d", "#101b30"),
    M("tunica_roja", "#e07a62", "#b04a38", "#7e3026", "#461a14", "#2e100c"),
    M("tunica_parda", "#c9a26a", "#9c7646", "#6e522f", "#3e2e1a", "#291e10"),
    M("tabardo_azul", "#5f86c8", "#3a5fa0", "#28437a", "#172a4d", "#0e1a30"),
    M("lino", "#f2ead2", "#d9ccaa", "#b3a47e", "#6e6248", "#4a412f"),
    M("chaleco_marron", "#9a6a44", "#744c30", "#523420", "#2e1d12", "#1f130b"),
    M("habito_purpura", "#6e5480", "#4c3a5c", "#342642", "#1d1426", "#120c18"),
    M("vestido_teja", "#d07a64", "#a8513f", "#77352a", "#421c16", "#2b110d"),
    M("lana_purpura", "#7a5a8c", "#553d66", "#3a2848", "#1f1428", "#140c1a"),
    M("pantalon_pardo", "#a07b52", "#7a5a3c", "#553d29", "#33241a", "#221710"),
    M("pantalon_gris", "#7d7f86", "#5b5d64", "#404248", "#25262a", "#17181b"),
    M("pantalon_oliva", "#8a8a5a", "#6a6a42", "#4a4a2e", "#2b2b1a", "#1c1c10"),
    # cuero, cuerda y metal
    M("cuero", "#b07a45", "#8a5a30", "#633f22", "#3a2412", "#26170b"),
    M("cuero_oscuro", "#7a5238", "#553624", "#3a2418", "#1e120b", "#140b06"),
    M("guante", "#8a6242", "#66462e", "#48301f", "#2a1b10", "#1c110a"),
    M("zapato", "#6e4a30", "#4e3320", "#362214", "#1e120b", "#140b06"),
    M("sandalia", "#5a4030", "#3e2a1e", "#2a1c14", "#160e09", "#0e0906"),
    M("cuerda", "#e0cfa0", "#c2ab74", "#96804e", "#54462a", "#382e1a"),
    M("laton", "#ffe28a", "#e0b040", "#a87a22", "#5a3c10"),
    M("oro_emblema", "#ffe98a", "#f0c030", "#b88a1a", "#6a4a0a"),
    M("acero", "#eef3f6", "#b4c0c8", "#7b8893", "#3a424a", "#24292e"),
    M("malla", "#c9d1d6", "#8f9aa3", "#5d6770", "#2e343a", "#1c2025"),
    # pelo
    M("pelo_castano", "#a0673a", "#7a4a28", "#56321c", "#2e1a0e", "#1f1109"),
    M("pelo_rubio", "#f0d890", "#d4b060", "#a8843a", "#5c4420", "#3e2c14"),
    M("pelo_negro", "#5a5a66", "#3a3a44", "#26262e", "#141418", "#0a0a0e"),
    M("pelo_pelirrojo", "#e8904a", "#c0602a", "#8a401c", "#4a200e", "#321508"),
    M("pelo_oscuro", "#8a5a3a", "#5e3a24", "#3e2416", "#22140a", "#160c06"),
    M("habito_gris", "#6a6a70", "#4a4a52", "#33333a", "#1a1a20", "#101014"),
    M("lana_gris", "#76767e", "#55555e", "#3a3a42", "#1e1e24", "#121216"),
    # piel (luz, base, sombra, contorno suave, contorno duro)
    M("piel_clara", "#ffebc6", "#fad2a9", "#d78b64", "#77432e", "#572e1d"),
    M("piel_morena", "#e8b98a", "#c98f5e", "#9a6440", "#5a3520", "#3e2414"),
    M("piel_palida", "#eee2d6", "#d6c4b4", "#a89080", "#5e4640", "#40302b"),
    M("piel_oscura", "#c08860", "#9a6440", "#6e4428", "#3e2414", "#28160a"),
    M("piel_aceituna", "#e8c090", "#c89a68", "#9a6e44", "#583a20", "#3a2412"),
    M("piel_rojiza", "#ffd0b0", "#f0a888", "#c07058", "#6a3428", "#48221a"),
    M("piel_gris", "#e0dcd8", "#bcb4b0", "#8a8080", "#4a4040", "#302828"),
    M("piel_elfo", "#fff4e0", "#f5dcc0", "#d0a888", "#7a5040", "#54362a"),
    M("piel_drow", "#8a80a8", "#665c84", "#48405e", "#26203a", "#161224"),
    M("piel_feerica", "#f8eaff", "#e4cdf0", "#b89acc", "#6a4a80", "#4a3060"),
    M("piel_orco", "#a6c878", "#7aa050", "#557538", "#2c3f1c", "#1c2912"),
    M("piel_orco_oliva", "#9ab060", "#728a40", "#50642c", "#2a3616", "#1a220c"),
    M("piel_trasgo", "#c0dc88", "#98bc60", "#6e9040", "#384c20", "#243214"),
    M("piel_tiefling", "#d6b4e0", "#b088c4", "#806096", "#43305a", "#2c1f3c"),
    M("piel_tiefling_oscura", "#6e5e80", "#4e4260", "#342c44", "#1a1424", "#0e0a14"),
    # las sagas son drows corrompidos (PNJ-02): la piel del drow, enferma de musgo (locura)
    # o helada de abismo; las luces tiran a su color y las sombras se quedan violetas
    M("piel_drow_palida", "#a3a399", "#7b7886", "#58526e", "#2e2842", "#1a1628"),
    M("piel_drow_azul", "#9cb2d4", "#6a74a4", "#4a4a7a", "#26244a", "#16142e"),
    M("piel_aparecido", "#b0c0d0", "#8898b0", "#607088", "#303a4a", "#1e2630"),
    M("fantasma", "#e0ffff", "#90e8f8", "#50b8e0", "#2080b0", "#106090", alfa=190),
    # pelajes, escamas y plumas
    M("pelaje_toro", "#b07a50", "#8a5634", "#633a22", "#3a2012", "#26140a"),
    M("pelaje_toro_claro", "#e8c090", "#c89460", "#98683c", "#4e3420", "#322214"),
    M("pelaje_gato", "#f0b070", "#d08040", "#a05a28", "#5a3014", "#3a1e0c"),
    M("pelaje_gato_gris", "#9a9aa8", "#6e6e7e", "#4c4c5a", "#26262e", "#16161c"),
    M("pelaje_zorro_negro", "#5a5060", "#3a3240", "#26202c", "#141018", "#0a080c"),
    M("pelaje_zorro_blanco", "#ffffff", "#eef2f8", "#c4cede", "#6a7a92", "#46546a"),
    M("pelaje_zorro", "#ff9a50", "#e06a28", "#a84a18", "#5a2408", "#3a1608"),
    M("escamas_verdes", "#a0c870", "#6e9a48", "#4a7034", "#243c18", "#16280e"),
    M("escamas_oliva", "#8a9a60", "#667440", "#48522c", "#262c16", "#181c0c"),
    M("escamas_rojas", "#f08060", "#c84a30", "#8a2a1c", "#4a120a", "#2e0a06"),
    M("escamas_hielo", "#e8f4ff", "#b0cef0", "#7896c8", "#384a70", "#222e48"),
    M("escamas_azules", "#80b0f0", "#4a78c8", "#2e4c90", "#16244a", "#0c1630"),
    M("escamas_naga_verde", "#90d080", "#58a050", "#3a7438", "#1c3c1c", "#102410"),
    M("plumas_pardas", "#c89868", "#9a6a40", "#6e4a2a", "#3a2412", "#26170b"),
    M("plumas_blancas", "#ffffff", "#e8ecf0", "#b8c4d0", "#5a6878", "#3a4450"),
    M("remeras_pardas", "#8a6a4a", "#5e4430", "#3e2c1e", "#20160c", "#140e08"),
    M("remeras_azules", "#a0b8e0", "#6a88c0", "#48609a", "#243458", "#161f38"),
    M("membrana_hada", "#fff0ff", "#f8c8f0", "#d898d8", "#8a5a9a", "#6a3a7a", alfa=185),
    M("nervio_hada", "#fff0ff", "#e8b0e8", "#b070c0", "#7a4a90", "#5a2a70", alfa=230),
    # telas y metales del equipo
    M("gambeson", "#f0e0b8", "#d4c090", "#a8946a", "#5e5038", "#3e3424"),
    M("tela_negra", "#5a5a64", "#3c3c46", "#2a2a32", "#16161c", "#0c0c10"),
    M("tela_blanca", "#ffffff", "#e8e8ee", "#b8bcc8", "#626874", "#40444e"),
    M("tela_roja_oscura", "#d0584a", "#a03a30", "#722620", "#3e1210", "#280a08"),
    M("tela_azul_noche", "#6a7ad0", "#4450a0", "#2e3674", "#171c40", "#0e112a"),
    M("tela_amarilla", "#fff08a", "#f0cc40", "#c0962a", "#6a4c10", "#46320a"),
    M("terciopelo_rojo", "#e0506a", "#b02848", "#7a1a32", "#420c1a", "#2a0810"),
    M("harapo", "#b0a488", "#8a7e64", "#645a46", "#363024", "#241f18"),
    M("remiendo", "#c8a070", "#a07848", "#76542e", "#40301a", "#2a1f10"),
    M("madera", "#c89860", "#a0703c", "#744e28", "#402a14", "#2a1c0c"),
    M("madera_oscura", "#8a6244", "#64442c", "#44301e", "#24180e", "#160e08"),
    M("hierro", "#c8ccd0", "#8e949a", "#62686e", "#32363a", "#1e2124"),
    M("bronce", "#f0c080", "#c89048", "#946430", "#4e3418", "#34220e"),
    M("oro_viejo", "#fff0a0", "#e8c050", "#b08828", "#5e4410", "#3e2c08"),
    M("plata", "#ffffff", "#d8dee8", "#a0aab8", "#545c68", "#363c46"),
    M("ceniza", "#8a8a94", "#5e5e6a", "#40404a", "#202026", "#121216"),
    M("cristal_azul", "#e0f4ff", "#88c8f8", "#4888d0", "#1e4478", "#122c50"),
    M("roble", "#b88858", "#8a6038", "#624226", "#342212", "#22160a"),
    M("paja", "#fff0a8", "#e8cc70", "#b89a48", "#6a5424", "#463816"),
    M("fieltro_verde", "#78a060", "#527a42", "#38582e", "#1c3018", "#12200e"),
    M("pluma_roja", "#ff8070", "#e04038", "#a02820", "#50100c", "#340a08"),
    M("gema_roja", "#ffb0b0", "#f03040", "#a01020", "#500810", "#34040a"),
    M("gema_azul", "#c0e8ff", "#40a0f0", "#2060b0", "#103060", "#081e40"),
    M("gema_verde", "#c0ffc8", "#40d070", "#20904a", "#104824", "#082e16"),
    M("porcelana", "#ffffff", "#f4f0ec", "#d0c8c4", "#7a706c", "#524a48"),
    M("hojas", "#a8d870", "#78b048", "#508030", "#284418", "#182c0e"),
    M("espuma", "#ffffff", "#fff4d8", "#e0c890", "#6a5a3a", "#463c26"),
    M("llama", "#fff8c0", "#ffd040", "#f09020", "#5a3010", "#3a200a"),
    M("pocion_verde", "#d0ffc0", "#60e070", "#30a048", "#205a30", "#143a1e"),
    M("tela_naranja", "#ffc080", "#e8904a", "#b0602a", "#5a3014", "#3a1e0c"),
    M("tela_azul_marino", "#6a88b8", "#44628e", "#2e4468", "#18243a", "#0e1626"),
    M("tela_turquesa", "#80d8d0", "#40a8a0", "#287870", "#143e3a", "#0c2826"),
    # rasgos: cuerno, hueso, pico, morros, interiores
    M("cuerno_claro", "#fff4d8", "#e8d8b0", "#b0a080", "#4a3f2e", "#2e271c"),
    M("cuerno_oscuro", "#8a7a8a", "#5a4a5e", "#3a2e40", "#1e1624", "#120c16"),
    M("cuerno_draco", "#e8d8c0", "#b8a080", "#806848", "#3a2e1e", "#241c12"),
    M("hueso", "#fffbe8", "#e6dcc0", "#b8aa88", "#6a5e48", "#453d2f"),
    M("pico", "#ffe070", "#f0b030", "#b87818", "#5a3a08", "#3a2604"),
    M("pata_ave", "#ffe070", "#f0b030", "#b87818", "#5a3a08", "#3a2604"),
    M("una", "#505058", "#383840", "#24242a", "#141418", "#0a0a0c"),
    M("pezuna", "#6a5a50", "#4a3c34", "#302620", "#181210", "#0e0a08"),
    M("morro_toro", "#f0c8a8", "#d8a080", "#a87058", "#4a2a1a", "#2a1810"),
    M("hocico_gato", "#fff0e0", "#f0d8c0", "#c0a890", "#c05060", "#803040"),
    M("oreja_rosa", "#ffc8c0", "#f0a098", "#c07870", "#6a3c38", "#4a2826"),
    M("oreja_roja", "#ff8a7a", "#d04a3a", "#902a22", "#4a1410", "#2a0a08"),
    M("oreja_celeste", "#e8f8ff", "#b0e0f8", "#78b0d8", "#3a6080", "#243e54"),
    M("garganta", "#f0e8b0", "#d0c890", "#a09a68", "#4a4630", "#2e2c1e"),
    M("garganta_hielo", "#ffffff", "#e0f0ff", "#b0c8e8", "#5a6e90", "#3a4a64"),
    M("vientre_naga", "#f0e0a0", "#d8c078", "#a88c50", "#4a3c20", "#2e2412"),
    M("espinas", "#f0a060", "#d07030", "#904a1c", "#4a2208", "#301404"),
    M("mechon_toro", "#6a4a30", "#4a3020", "#301e12", "#180e08", "#0e0804"),
    M("pica_tiefling", "#8a6a9a", "#6a4a7a", "#4a3058", "#281830", "#180e1e"),
    # PNJ-02: la corrupcion de los drows (actores_corrupcion)
    M("corteza", "#9a7a5a", "#6e5440", "#4c3a2c", "#2a2018", "#1a140e"),
    M("musgo", "#b4cc6a", "#7e9e42", "#56742c", "#2c3e16", "#1c280c"),
    M("obsidiana", "#9a8ab8", "#4e4466", "#342c48", "#1a1426", "#0e0a16"),
    M("brillo_violeta", "#f4dcff", "#d0a0f0", "#9a60c8", "#4a2a70", "#301a4a"),
    # Oriol (23/10): las marcas de Egaroth, el planeta y padre elemental, en la piel de las razas que la
    # tocó su energía (manchas de colores: actores_rasgos.MANCHAS_*)
    M("egaroth_dormienta", "#e2f0ff", "#9cc0e8", "#5e86b8", "#2e4870", "#1c2e4a"),
    M("egaroth_ambar", "#fff0b8", "#f6c25a", "#d08a2a", "#7a4a14", "#4e2e0c"),
    M("egaroth_turquesa", "#d8fff4", "#7ee8d0", "#3ab49e", "#1a6458", "#0e3e36"),
    M("egaroth_violeta", "#f6e2ff", "#d6a8f8", "#a46ad8", "#523080", "#341e54"),
    # Oriol (23/10): «una altra subraça per cada nació del 2000 b.f.»: más colores de Egaroth, unos más
    # oscuros, y la piel de los pueblos nuevos (actores_razas, «2000 b.f.»)
    M("egaroth_sol", "#fffbe0", "#ffe27a", "#f0b43a", "#8a5a12", "#5a3a0a"),
    M("egaroth_perla", "#ffffff", "#f4eefa", "#d0c6e0", "#7a6e90", "#524864"),
    M("egaroth_cobalto", "#d8e8ff", "#5a8af0", "#2e56c0", "#142a6a", "#0c1a44"),
    M("egaroth_jade", "#e0ffe0", "#7ae09a", "#36a868", "#145a34", "#0c3a20"),
    M("egaroth_brasa", "#fff0c0", "#ff9a40", "#e05020", "#7a1e0c", "#4e1206"),
    M("egaroth_tinta", "#6a6a88", "#2e2e44", "#1e1e30", "#0e0e18", "#08080e"),
    M("egaroth_ceniza", "#a8a0a0", "#5e5656", "#403a3a", "#201c1c", "#141010"),
    M("egaroth_sangre", "#ffb0a0", "#d04040", "#902428", "#481014", "#2e080a"),
    M("plumas_verdes", "#c8e8a0", "#86b85a", "#5a8a3a", "#2e4a1c", "#1e3010"),
    M("remeras_verdes", "#a0d870", "#5a9a3a", "#3a6a26", "#1c3812", "#12240a"),
    M("pelaje_toro_gris", "#b8b8c0", "#8a8a94", "#62626c", "#32323a", "#202026"),
    M("escamas_tormenta", "#8aa8e0", "#3e5aa0", "#2a3c74", "#141e40", "#0c1228"),
    M("escamas_negras", "#7a7a86", "#3c3c48", "#282832", "#121218", "#08080c"),
    M("piel_oni", "#ffb8a0", "#e07060", "#a84438", "#5a1e18", "#3a120e"),
    M("piel_marina", "#e0f4ff", "#a8d4e8", "#6aa0c0", "#2e5470", "#1e3a4e"),
    M("piel_brasa", "#ffd8b0", "#f0a070", "#c06a40", "#6a321a", "#462010"),
    M("piel_saga", "#d8e0c0", "#a8b48a", "#788660", "#3c4430", "#262c1e"),
    M("piel_orco_ceniza", "#b8b8a8", "#8a8a7a", "#5e5e52", "#2e2e28", "#1e1e1a"),
    M("piel_gnomo", "#ffe0c0", "#f4c09a", "#d08a64", "#7a4a30", "#56321e"),
    M("agua_elemental", "#f0ffff", "#80d0f8", "#3a90d8", "#1a4a90", "#0e2e60", alfa=215),
    M("roca_elemental", "#c8b8a0", "#8e7c66", "#625444", "#322a22", "#201a14"),
    M("aire_elemental", "#ffffff", "#e0f0f8", "#b0cce0", "#6a8aa8", "#4a6886", alfa=165),
    M("pelo_llama", "#fffae0", "#ffd860", "#f09a30", "#a04a10", "#6a2e08"),
    M("piedra_abismo", "#7e8898", "#454c5c", "#2c3240", "#15181f", "#0a0b10"),
    M("ojo_abismo", "#e8fbff", "#9ae6ff", "#4ab0e0", "#1a5a80", "#0e3a54"),
    # pelo de raza
    M("pelo_blanco", "#ffffff", "#e4e4ec", "#b4b4c4", "#5e5e70", "#3e3e4c"),
    M("pelo_gris", "#c8c8cc", "#9c9ca4", "#707078", "#38383e", "#242428"),
    M("pelo_plata_lila", "#fff0ff", "#e8d0f0", "#b8a0cc", "#6a5080", "#4a3460"),
    M("pelo_verde_musgo", "#a8c870", "#7a9a48", "#566e32", "#2c3a18", "#1c260e"),
    M("pelo_azul_hielo", "#f0f8ff", "#c0e0f8", "#88b4e0", "#3e6088", "#283e5a"),
    # ESTETICA-01: las veinte esencias de las armas (la hoja de la daga de plaga...);
    # ceniza, cristal azul, oro viejo y plata ya estaban
    M("roble_antiguo", "#a89060", "#76603a", "#50402a", "#2a2016", "#1a140c"),
    M("hierro_santo", "#fff2c0", "#cdc8b8", "#938c80", "#4a4640", "#2e2b27"),
    M("luna", "#f6f0ff", "#c8bce0", "#9488b4", "#4c4466", "#302a44"),
    M("niebla", "#f4f8fb", "#c4ced6", "#97a3ad", "#5a666f", "#3c464e"),
    M("penumbra", "#8a80a0", "#5a5070", "#3c344e", "#1e1a28", "#120f18"),
    M("plaga", "#d8e890", "#9cb050", "#6a7a30", "#343c16", "#20260c"),
    M("trueno", "#fffbd0", "#a0c8f0", "#5a7cc0", "#22346a", "#141f44"),
    M("tumba", "#e8e8d0", "#b0b098", "#7a7c6a", "#3e4036", "#262820"),
    # CP-01: la luz verde palida de las animas (el farol y las velas del Necromago); aqui y no en su set
    # porque el farol es un arma (actores_formas_propias) y las armas se pintan sin los trajes
    M("luz_animas", "#f4fff0", "#bdf7ae", "#74d46c", "#2f6b2c", "#1f471d"),
    M("estrella", "#ffffff", "#4a5aa8", "#2e3878", "#161c40", "#0c1028"),
    M("juramento", "#fffaf0", "#f0e0b0", "#c8a868", "#6a5228", "#463418"),
    M("rey_lobo", "#ffffff", "#c8e0f0", "#88a8c8", "#3a5068", "#243444"),
    M("sangre", "#ff9a8a", "#c0302c", "#80181c", "#40080c", "#280406"),
    M("sombra_roja", "#ff5a48", "#3a2228", "#24141a", "#120a0e", "#080406"),
    M("corona_rota", "#f0e0a0", "#b8a050", "#6a7040", "#3a3418", "#24200e"),
    M("eternidad", "#ffffff", "#d8fff8", "#8ae0e0", "#2a8a96", "#185a64"),
    M("vacio", "#d8a0ff", "#2a1e3a", "#1a1226", "#0c0812", "#050308"),
    # ESTETICA-01: metales, cueros y telas de los temas (deidades, nombres) y de los acabados
    M("oro_rojo", "#ffe0c8", "#e8a878", "#b07048", "#5a3420", "#3a2014"),
    M("hierro_forja", "#ffb060", "#6a6a70", "#48484e", "#24242a", "#141418"),
    M("hierro_negro", "#9098a0", "#50565e", "#363a40", "#1a1c20", "#0e1012"),
    M("hierro_viejo", "#c8a890", "#8a6a58", "#604838", "#34241c", "#221610"),
    M("acero_tormenta", "#e8f4ff", "#8aa8c8", "#5a7898", "#2a3c52", "#1a2636"),
    M("bronce_marino", "#c8f0d8", "#6ab0a0", "#3e8078", "#1c4440", "#102c2a"),
    M("plata_negra", "#d0d0e0", "#7a7a90", "#54546a", "#2a2a3a", "#1a1a26"),
    M("vidrio", "#f0ffff", "#a8e8f0", "#60b0c8", "#2a6078", "#1a3e50"),
    M("malla_plata", "#ffffff", "#d0d8e0", "#a0a8b4", "#50586a", "#343a46"),
    M("malla_oro", "#fff0b0", "#d8b860", "#a08438", "#54421a", "#382c10"),
    M("cuero_rojo", "#c86a5a", "#8a3a30", "#5e2420", "#321210", "#200b0a"),
    M("cuero_negro", "#6a5a5a", "#3a3030", "#261e1e", "#120e0e", "#0a0808"),
    M("cuero_polvo", "#b8a890", "#8a7a64", "#645846", "#363026", "#241f18"),
    M("cuero_leon", "#f8d890", "#d0a050", "#9a7030", "#50381a", "#34240e"),
    M("fresno", "#e8d8b0", "#c8b080", "#9a8458", "#54462e", "#382e1e"),
    M("pergamino", "#fff8e0", "#ecdcb0", "#c8b080", "#6a5a3a", "#463c26"),
    M("lana_lila", "#f0e8ff", "#c8b8e0", "#9a88b8", "#54486a", "#383048"),
    M("perla", "#ffffff", "#f0ece8", "#c8c0c8", "#6a6070", "#48404c"),
    M("gema_violeta", "#f0d0ff", "#b060e0", "#7030a0", "#3a1658", "#240c3a"),
    M("pelaje_lobo", "#d8d8e0", "#a0a0ac", "#707080", "#383844", "#22222c"),
    M("verde_serpiente", "#a0f0b0", "#30a060", "#1c6a40", "#0c3420", "#062214"),
    M("tela_verde_oscura", "#6a9a5a", "#466e3c", "#2e4c28", "#182a14", "#0e1a0c"),
    M("tela_gris_azul", "#8a9aac", "#62707e", "#465260", "#242c34", "#161c22"),
    M("tela_purpura_real", "#b080d0", "#7a4aa0", "#54307a", "#2c1844", "#1c0e2e"),
    M("tela_marfil", "#fffcf0", "#f0e8d0", "#c8bc9c", "#6e6450", "#4a4234"),
    M("tela_ocre", "#f0c870", "#c89a40", "#946e28", "#4e3812", "#34240a"),
    M("cuarzo_ahumado", "#f0e8e0", "#a89890", "#786a64", "#3e3432", "#282120"),
    M("cuero_azul", "#8aa0c8", "#5a6e98", "#3e4c70", "#1e2640", "#121828"),
    M("cuero_turquesa", "#9ae0d8", "#4aa8a0", "#2e7a74", "#163e3a", "#0e2826"),
    M("cuero_blanco", "#fffaf0", "#e0d8c8", "#b0a490", "#5e5444", "#3e382c"),
    M("cuero_verde", "#9ab07a", "#6a804e", "#4a5a34", "#24301a", "#161e10"),
    M("malla_espuma", "#ffffff", "#c8ece8", "#90c8c4", "#3e6a6a", "#284646"),
    M("cuero_ceniza", "#a0a0a8", "#707078", "#505058", "#28282e", "#18181c"),
    M("cuero_violeta", "#a08ab8", "#6a5488", "#4a3a62", "#241c34", "#161020"),
    M("cuero_brasa", "#f0a060", "#b8602c", "#80401c", "#42200c", "#2a1408"),
    M("cuero_vino", "#c0607a", "#842a48", "#5a1a32", "#2e0c1a", "#1e0810"),
    M("cuero_pizarra", "#9aa4b0", "#6a7480", "#4c545e", "#262a30", "#16191d"),
    # tinta y pluma de cuervo: negro con el lustre azul
    M("tinta_negra", "#7088b8", "#2e3448", "#1e2130", "#0f1018", "#07080c"),
]}

OJOS = {
    "azul": {},  # los del cuerpo base
    "negro": {"i1": (16, 14, 18), "i2": (34, 30, 36), "i3": (64, 58, 66)},
    "amarillo": {"i1": (40, 30, 8), "i2": (200, 170, 20), "i3": (250, 230, 90)},
    "rojo": {"i1": (110, 10, 10), "i2": (200, 40, 30), "i3": (250, 110, 80)},
    "violeta": {"i1": (60, 20, 90), "i2": (130, 60, 180), "i3": (200, 140, 240)},
    "dorado": {"i1": (120, 70, 0), "i2": (220, 160, 20), "i3": (255, 230, 120)},
    "blanco": {"i1": (160, 220, 240), "i2": (210, 245, 255), "i3": (255, 255, 255)},
    "marron": {"i1": (46, 28, 18), "i2": (82, 50, 28), "i3": (120, 78, 42)},
    "verde": {"i1": (20, 70, 40), "i2": (30, 110, 60), "i3": (60, 150, 80)},
    # brillo ambar bajo la capucha: el blanco tambien queda en penumbra
    "ambar": {"i1": (160, 90, 10), "i2": (235, 160, 30), "i3": (255, 225, 110),
              "w1": (86, 64, 92), "w2": (98, 74, 104), "w3": (110, 86, 116)},
}


# ESTETICA-01: el acabado de un material ("<material>_insigne", "<material>_sublime"),
# hecho a partir de el cuando un objeto lo pide (actores_estetica). Telas, cueros y
# maderas: Insigne es el mismo tinte mas vivo con la luz plateada; Sublime, mas hondo y
# con la luz dorada. Metales: el mismo metal mezclado con plata o con oro.
def _rgb_hsv(c, ks, kv, hacia=None, peso=0.0):
    import colorsys
    h, sat, v = colorsys.rgb_to_hsv(*[x / 255 for x in c])
    r, g, b = colorsys.hsv_to_rgb(h, min(1.0, sat * ks), min(1.0, v * kv))
    out = [r * 255, g * 255, b * 255]
    if hacia:
        out = [a * (1 - peso) + b_ * peso for a, b_ in zip(out, hacia)]
    return "#%02x%02x%02x" % tuple(int(round(x)) for x in out)


_PLATA_LUZ, _ORO_LUZ = (238, 244, 255), (255, 226, 122)


def acabado(nombre, cual, metal=False):
    """Nombre del material con el acabado `cual` ('insigne' o 'sublime'); lo crea si no esta."""
    nuevo = f"{nombre}_{cual}"
    if nuevo in MATERIALES:
        return nuevo
    m = MATERIALES[nombre]
    t = [m.tono[k] for k in ("H", "s", "d", "o", "O")]
    if metal:
        meta = MATERIALES["plata" if cual == "insigne" else "oro_viejo"]
        peso = 0.4 if cual == "insigne" else 0.5
        tonos = ["#%02x%02x%02x" % tuple(int(round(a * (1 - peso) + b * peso)) for a, b in zip(c, meta.tono[k]))
                 for c, k in zip(t, ("H", "s", "d", "o", "O"))]
    elif cual == "insigne":
        tonos = [_rgb_hsv(t[0], 1.2, 1.0, _PLATA_LUZ, 0.35), _rgb_hsv(t[1], 1.3, 1.04), _rgb_hsv(t[2], 1.3, 1.02),
                 "#%02x%02x%02x" % t[3], "#%02x%02x%02x" % t[4]]
    else:
        tonos = [_rgb_hsv(t[0], 1.2, 1.0, _ORO_LUZ, 0.35), _rgb_hsv(t[1], 1.3, 0.8), _rgb_hsv(t[2], 1.3, 0.78),
                 _rgb_hsv(t[3], 1.0, 0.85), "#%02x%02x%02x" % t[4]]
    MATERIALES[nuevo] = M(nuevo, *tonos, alfa=m.alfa)
    return nuevo
