typedef char *textcolls_t;
//typedef textcolls_t[12] txtcol_t;

typedef struct
{
	char *title;
	int num_colls;
	int num_collss;
	char imageName[12];
	int x,y;
	textcolls_t *colls;
	textcolls_t *colls_side;
	int collOffset;
} journalPage_t;

typedef struct
{
	int num_pages;
	journalPage_t *pages;
} journalPages_t;

static char *maxhp;
static textcolls_t statColls1[]=
{
	{"HITPOINTS:"},
	{""},
	{""},
	{""},
	{""},
	{""},
	{""},
	{""},
	{""},
	{""},
	{""},
	{""},
	{""}
};

/*static textcolls_t statColls2[]=
{
	{"HE IS A MAGE"},
	{"HE CAN DO IT ALL AND MORE"}
};*/

static journalPage_t statPages[]=
{
	{"YOUR STATISTICS",12,0,"",185,99,statColls1,NULL,30}
//	{"HIS STATISTICS",2,0,"MAGEB1",188,99,statColls2,NULL,110}
};

static textcolls_t bestiaryColls1[]=
{
	{"THESE HORRID CREATURES WERE"},
	{"ONCE PROUD MEMBERS OF THE"},
	{"LEGION. THEY HATE HUMANS FOR"},
	{"BEING SOMETHING THEY CAN'T BE."},
};

static textcolls_t bestiaryColls1s[]=
{
	{"HIT POINTS:"},
	{"130"},
	{""},
	{"ATTACK DAMAGE:"},
	{"2-16"}
};

static textcolls_t bestiaryColls2[]=
{
	{"ESSENTIALLY A FIRE GARGOYLE,"},
	{"IT'S MORE THAN HAPPY TO DOUSE"},
	{"YOU IN FLAMES. AND UNLIKE YOU,"},
	{"THIS BEAST CAN FLY."}
};

static textcolls_t bestiaryColls2s[]=
{
	{"HIT POINTS:"},
	{"80"},
	{""},
	{"ATTACK DAMAGE:"},
	{"UP TO 8"}
};

static textcolls_t bestiaryColls3[]=
{
	{"THE SPINE OF KORAX' ARMIES,"},
	{"THESE MYTHICAL HORSE-MEN ARE"},
	{"VICIOUS, BRUTAL AND HARD TO"},
	{"KILL. CAN DEFLECT MISSILES."}
};

static textcolls_t bestiaryColls3s[]=
{
	{"HIT POINTS:"},
	{"200"},
	{""},
	{"ATTACK DAMAGE:"},
	{"UP TO 10"}
};

static textcolls_t bestiaryColls4[]=
{
	{"ONCE THEY APPEAR, NO FOREST"},
	{"CAN BE CALLED PEACEFUL ANY"},
	{"MORE. THEIR BREATHING IS FIRE."}
};

static textcolls_t bestiaryColls4s[]=
{
	{"HIT POINTS:"},
	{"250"},
	{""},
	{"ATTACK DAMAGE:"},
	{"5-40"}
};

static textcolls_t bestiaryColls5[]=
{
	{"A BROWN MENACE OF MOST DESERTS"},
	{"ON CRONOS, THESE POISON-"},
	{"BELCHING MONSTERS ARE A TOUGH"},
	{"CHALLENGE FOR ALL BUT THE BEST."}
};

static textcolls_t bestiaryColls5s[]=
{
	{"HIT POINTS:"},
	{"500"},
	{""},
	{"ATTACK DAMAGE:"},
	{"10-80"}
};

static textcolls_t bestiaryColls6[]=
{
	{"AN UNDEAD DRAGON, ITS THIRST"},
	{"FOR THE BLOOD OF THE LIVING IS"},
	{"SURPASSED ONLY BY ITS HATRED"},
	{"OF ANYTHING BUT ITSELF."}
};

static textcolls_t bestiaryColls6s[]=
{
	{"HIT POINTS:"},
	{"640"},
	{""},
	{"ATTACK DAMAGE:"},
	{"UP TO 128"}
};

static journalPage_t bestiaryPages[]=
{
//	{"title",#oflines,"pic",xoffsetpic,yoffsetpic,IDofbodytext,yoffsetbodytext},
	{"ETTIN",4,5,"ETTNG2",123,99,bestiaryColls1,bestiaryColls1s,110},
	{"AFRIT",4,5,"FDMND1",123,99,bestiaryColls2,bestiaryColls2s,110},
	{"CENTAUR",4,5,"CENTG8",123,99,bestiaryColls3,bestiaryColls3s,110},
	{"FOREST CHAOS SERPENT",3,5,"DEMNE2E8",142,129,bestiaryColls4,bestiaryColls4s,140},
	{"WASTELAND CHAOS SERPENT",4,5,"DEM2G1",142,129,bestiaryColls5,bestiaryColls5s,140},
	{"DEATH WYVERN",4,5,"DRAGB2B8",135,109,bestiaryColls6,bestiaryColls6s,110}
};

static textcolls_t itemColls1[]=
{
	{"THIS SIMPLE ITEM LETS YOU"},
	{"SEE BETTER IN DARK PLACES."},
	{"ONCE LIT, IT'LL BE USED UP"},
	{"WITHIN A SHORT TIME."}
};

static textcolls_t itemColls2[]=
{
	{"FOUND FLOATING IN THE AIR IN"},
	{"SOME PLACES, THIS SMALL VIAL"},
	{"RESTORES A SMALL PART OF YOUR"},
	{"LIFE FORCE, HEALING WOUNDS IN"},
	{"THE PROCESS."}
};

static textcolls_t itemColls3[]=
{
	{"FOUND FLOATING IN THE AIR IN"},
	{"SOME PLACES, THIS SMALL VIAL"},
	{"RESTORES A LARGE PART OF YOUR"},
	{"LIFE FORCE, HEALING WOUNDS IN"},
	{"THE PROCESS."}
};

static textcolls_t itemColls4[]=
{
	{"THIS MAGICAL ITEM INCREASES"},
	{"THE AMOUNT OF BLUE MANA YOUR"},
	{"WEAPONS HAVE AT THEIR DISPOSAL."}
};

static textcolls_t itemColls5[]=
{
	{"THIS MAGICAL ITEM INCREASES"},
	{"THE AMOUNT OF GREEN MANA YOUR"},
	{"WEAPONS HAVE AT THEIR DISPOSAL."}
};

static textcolls_t itemColls6[]=
{
	{"THIS MAGICAL ITEM INCREASES"},
	{"THE AMOUNT OF ALL MANA YOUR"},
	{"WEAPONS HAVE AT THEIR DISPOSAL."}
};

static textcolls_t itemColls7[]=
{
	{"THIS MAGICAL ITEM INCREASES"},
	{"THE AMOUNT OF ALL YOUR MANA"},
	{"TO THE MAXIMUM."}
};

static textcolls_t itemColls8[]=
{
	{"THIS LIGHT ARMOR IS MOST"},
	{"USEFUL TO THE FIGHTER AND"},
	{"LEAST USEFUL TO THE MAGE."}
};

static textcolls_t itemColls9[]=
{
	{"FROM ALL THE HEROES, A CLERIC"},
	{"TAKES OUT THE MOST PROTECTION"},
	{"FROM THIS HOLY SHIELD."}
};

static textcolls_t itemColls10[]=
{
	{"FROM THIS MAGICAL DEFENSIVE"},
	{"ITEM, A MAGE PROFITS MOST AND"},
	{"A FIGHTER THE LEAST."}
};

static textcolls_t itemColls11[]=
{
	{"THIS HELM OFFERS ONLY A"},
	{"SLIGHT PROTECTION AGAINST"},
	{"MONSTERS, BUT IT'S STILL"},
	{"BETTER TO BE ON THE SAFE SIDE."}
};

static textcolls_t itemColls12[]=
{
	{"UNLIKE TRADITIONAL ARMOR,"},
	{"THESE MAGICAL BRACERS WILL"},
	{"LOOSE THEIR POWER AFTER A"},
	{"SHORT TIME, REGARDLESS WHETHER"},
	{"THEY TAKE ANY DAMAGE."}
};

static textcolls_t itemColls13[]=
{
	{"A MAGICAL BOMB. THROW IT TO"},
	{"USE. PERFORMS DIFFERENTLY IN"},
	{"THE HANDS OF HEROES FROM"},
	{"DIFFERENT ORDERS."}
};

static textcolls_t itemColls14[]=
{
	{"WHEN YOU USE THIS ARTIFACT,"},
	{"EVERYTHING WITHIN A CERTAIN"},
	{"RADIUS OF YOUR BODY IS DEFLECTED"},
	{"AWAY, INCLUDING MONSTERS, SPELLS,"},
	{"AND EVEN PROJECTILES."}
};

static textcolls_t itemColls15[]=
{
	{"TYPICALLY FOUND IN MINES, THIS"},
	{"TREASURE WILL SELL FOR A VERY"},
	{"HIGH PRICE IN ANY TOWN."}
};

static textcolls_t itemColls16[]=
{
	{"TYPICALLY FOUND IN MINES, THIS"},
	{"TREASURE WILL SELL FOR A HIGH"},
	{"PRICE IN ANY TOWN."}
};

static textcolls_t itemColls17[]=
{
	{"USED AT THE MAKING OF HOLY"},
	{"ARTIFACTS THAT OFFER DEFENSE"},
	{"AGAINST THE UNDEAD, THIS SKULL"},
	{"WILL FETCH A GOOD PRICE IN ANY"},
	{"TEMPLE."}
};

static textcolls_t itemColls18[]=
{
	{"AN ESSENTIAL INGREDIENT FOR"},
	{"MANY A SPELL OR MAGICAL POTION,"},
	{"ANY MAGE WILL PAY YOU A GOOD"},
	{"PRICE FOR SUCH AN ITEM."}
};

static textcolls_t itemColls19[]=
{
	{"PUTTING THESE BOOTS ON WILL"},
	{"GIVE YOU AN IMMENSE SPEED BOOST,"},
	{"UNTIL THEY WEAR OUT AFTER A"},
	{"SHORT TIME."}
};


static journalPage_t itemPages[]=
{
//	{"title",#oflines,"pic",xoffsetpic,yoffsetpic,IDofbodytext,yoffsetbodytext},
	{"TORCH",4,0,"TRCHB0",185,99,itemColls1,NULL,110},
	{"HEALTH VIAL",5,0,"PTN1A0",185,99,itemColls2,NULL,110},
	{"QUARTZ FLASK",5,0,"PTN2A0",185,99,itemColls3,NULL,110},
	{"BLUE MANA",3,0,"MAN1H0",185,99,itemColls4,NULL,110},
	{"GREEN MANA",3,0,"MAN2P0",185,99,itemColls5,NULL,110},
	{"COMBINED MANA",3,0,"MAN3H0",185,99,itemColls6,NULL,110},
	{"KRATER OF MIGHT",3,0,"BMANA0",185,99,itemColls7,NULL,110},
	{"MESH ARMOR",3,0,"ARM1A0",185,99,itemColls8,NULL,110},
	{"FALCON SHIELD",3,0,"ARM2A0",185,99,itemColls9,NULL,110},
	{"AMULET OF WARDING",3,0,"ARM4A0",185,99,itemColls10,NULL,110},
	{"PLATINUM HELM",4,0,"ARM3A0",185,99,itemColls11,NULL,110},
	{"DRAGONSKIN BRACERS",5,0,"BRACC0",185,99,itemColls12,NULL,110},
	{"FLECHETTE",4,0,"THRWA0",185,99,itemColls13,NULL,110},
	{"DISC OF REPULSION",5,0,"BLSTA0",185,99,itemColls14,NULL,110},
	{"GOLD BAR",3,0,"RADEI0",185,99,itemColls15,NULL,110},
	{"SILVER BAR",3,0,"RADEI0",185,99,itemColls16,NULL,110},
	{"HUMAN SKULL",5,0,"RADEI0",185,99,itemColls17,NULL,110},
	{"ANIMAL SKULL",4,0,"ASKUA0",185,99,itemColls18,NULL,110},
	{"BOOTS OF SPEED",4,0,"SPEDB0",185,99,itemColls19,NULL,110},
};

static journalPages_t journalPages[5]=
{
	{1,statPages},
	{6,bestiaryPages},
	{19,itemPages}
};

