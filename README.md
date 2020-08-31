# SAS - Smart Arduino Signal
Een Smart Arduino Signal of SAS is een printplaat met onder anderen een arduinoboard die bedoeld is om niet alleen een sein aan te sturen maar ook om de modelbaan te beveiligen. De SAS is zodoende instaat om automatisch treinen te beinvloeden door middel van verschillende insteekbare modules. Er is een relaismodule om de baanspanning af te schakelen tbv de veiligheid. Er zijn drie type remmodules mogelijk om digitale treinen meet te beinvloeden: DC braking, Lenz ABC type 1 en Lenz ABC type 2. De SAS heeft verder aansluitingen om te communiceren met naburige seinen en om de SAS kan verkeerd spoor rijden.

Met SAS zou elke gebruiker uit de voeten kunnen komen. Je kan SAS gebruiken puur voor het cosmetische effect van een sein. Een SAS kan een baan beveiligen door een sectie af te schakelen en SAS kan zowel digitale en analoge treinen automatisch beinvloeden. Of je nu analoog of digitaal rijdt, of je nu seinen wilt voor de sier, beveiliging of automatisme, dan is SAS handig voor jou.

Een bloksysteem van seinen met een enkel bereden spoor is natuurlijk niet zo bijzonder ingewikkeld. Het ingewikkelde gedeelte dat zijn de dubbelbereden sporen, wisselstraten en stations. SAS is ook ontworpen om al deze problemen te tackelen. SAS kan stations beveiligen en SAS dubbel bereden enkelsporen beveiligen.

## Basis werking

Als een SAS niet is verbonden met een naburig sein, dan werkt de SAS standaard met de bezetmelder en met een afval vertraging. Als een trein de bezetmelder verlaat, dan gaat er een tijd lopen. Als deze tijd verstreken is, kan de SAS op groen of op geel springen afhankelijk of de SAS is ingesteld als hoofdsein of combinatiesein. In het geval van een combinatiesein, zal SAS naar nog een keer dezelfde tijd ook een groen seinbeeld aannemen. Dit betekent dat je behalve voeding, slechts een bezetmelder hoeft aan te sluiten. 

De SAS kan werken met zowel volledig gedecteerde stukken als met korte gedetecteerde stukken, hoewel het over het algemeen beter is om volledige detectie te gebruiken. Een SAS kan ook communiceren met naburige seinen. Als de verbinding gelegd is, zal de SAS niet meer op tijd werken maar op basis van signalen. Als de bezetmelder verbroken is en het volgende sein springt op rood dan zal de SAS weer op groen springen omdat hij begrijpt dat de trein op de bezetmelder van het volgende blok is gekomen.


## Voor wie is SAS bedoeld?
SAS is bedoeld voor:
- analoog rijders
- twee rail rijders
- drie rail rijders
- digitaal rijders
- computer rijders
- vaste banen
- module banen

## Wat heeft SAS allemaal aan boord?
Een SAS heeft de volgende attributen aan boord:
- een Arduino board
- een schakelende DC-DC voeding
- een gelijkrichter indien men een AC spanning wilt gebruiken als voeding
- een DCC interface
- elektronica voor bezetmelding op basis van stroom detectie
- mogelijkheden om ook bezetmelding te realiseren op basis van massa detectie of infrarood sensors
- drie open collector aansluiting een een V+ aansluiting om drie standaard of zelfbouw lichtseinen op aan te sluiten
- een connector voor een servo motor aansluiting
- een connector voor drie eventuele drukknopjes tbv extra aansturing van het sein.
- Een 4 polige DIP switch om de SAS te vertellen wat voor type sein hij is.
- Er is ook een extra connector om een printplaat met een potmeter en een knop op aan te sluiten om de SAS in te regelen
- een potmeter of een eventuele automatische afval in te stellen

## Wat kan een SAS allemaal?
Wat SAS kan:
- Een bezetmelder naar keuze inlezen
- Communiceren met naburige seinen om correcte seinbeelden te tonen en door te geven
- Allerlei seinbeelden conform de Nederlands en Duitse seinbeelden nabootsen
- Van achter langs gepasseerd worden
- Een servo motor aansturen met massatraagheid functionaliteit ingebouwd. Standaard draait de servo van 45 naar 135 graden. Dit is inregelbaar
- Elke lichtsein's helderheid kan individueleel worden ingesteld. Standaard staan de waardes op 100%
- Seinen in en uit laten faden om gloeilampen na te bootsen.
- fungeren als voorsein, hoofdsein of combinatiesein
- van een rood seinbeeld naar een groen of geel seinbeeld springen op basis van een instelbare tijd
- een sectie stroomloos schakelen achter een sein om het spoor te beveiligen.
- gelockt worden als bijvoorbeeld een wissel niet goed staat
- automatisch een digitale trein beinvloeden dmv drie verschillende type remmodules (Marklin DC braking, ABC type 1 en ABC type 2)
- stations beveiligen 
- Het seinbeeld rijden op zicht tonen.


## Q & A
Q: Hoe ben je op dit idee gekomen?
A: Ik dacht met BMB leden mee over een eventuele beveiliging van hun modulebaan. Zij kampten met verscheidene problemen; niet iedereen wilde zijn bak van een beveiliging voorzien, de opstelling van de baan is elke keer anders en moest allemaal maar stupid simple blijven. De blokbeveiliging wilde ik toen tackelen door een PCB te ontwerpen met een arduino board met 2 RJ45 aansluitingen om zo de communicatie tussen de modules op te zetten. Als er tussen 2 arduino uitgeruste modules een module zonder arduino zou liggen, dan zou men de bakken alsnog gemakkelijk kunnen verbinden met een ethernetkabel. Als een bak te laatse zou zijn, dan zou de bak verder kunnen beveiligen door of op tijd te schakelen of door zijn uitgaande signaal door te lussen naar zijn ingaande signaal. Dat zou dus betekenen dat de allerlaatste bak pas een 2e trein toe zou laten nadat de eerste trein via de keerlus weer terug is gekomen. Dit zou dan ook hun keerlusbeveiliging oplossen. Uiteindelijk waren er meer leden tegen dan voor en was het niks geworden.

Q: Heb je dit alleen gemaakt?
A: Nee ik heb met Timo erg veel contact gehad omtrent het ontwerp. Timo heeft echt waardevolle tips en adviezen gegeven om niet alleen een goede werking maar ook om een goede gebruikerservaring te realiseren.

Q: Is er eigenlijk iets wat SAS niet kan?
A: Er is een ding wat een SAS niet kan. Een inrijsein kan groen of geel seinbeeld tonen afhankelijk wat de stand is van de wissels die er achter liggen. Een SAS zal simpwel groen tonen mits het geplande spoor niet bezet is of de wissels gebogen staan of niet.

# De werking nader uitgelegd, hoe sluit je een SAS aan?

## Blokbeveiliging van een enkel bereden dubbelsporig hoofdlijn.
plaatjes

## Blokbeveiliging van een dubbel bereden dubbelsporig hoofdlijn.

plaatjes

---
## Blokbeveiliging van een enkel sporig branchlijn met uni-directionele passeerstukken
Hier volgt mogelijke aansluit mogelijkheden om SAS modules aan te sluiten voor een enkelsporig branchlijntje waarbij de passeer stukken in slechts een richting bereden kunnen worden.

In het eerste voorbeeld heeft elke SAS module slechts 1 bezet melder die het hele volgende blok detecteert. Dit is eigenlijk hetzelfde principe als het het eerste voorbeeld van een dubbel hoofdspoor. Wat hier is toegevoegd is een dubbeluitgevoerde rijrichtinglijn. Met 1 wisselschakelaar wordt 1 van de 2 lijnen altijd naar de 0V getrokken. Dit zorgt er voor dat de inrijseinen van achter bereden kunnen worden en dat de uitrijseinen nu alleen groen vertonen als het volgende blok vrij is en de rijrichting is goed ingesteld.

![alt text](https://raw.githubusercontent.com/bask185/SAS/master/images/img1-1.png "1-1")

Bovenstaand voorbeeld is de meest simpele doch effectieve manier. Wat hier echter aan nog ontbreekt zijn wisselbeveiligingen. Natuurlijk zou je magneetwissels mee kunnen schakelen met de rijrichting lijnen (mits de wissels een eindafschakeling hebben) dan weet je ook zeker dat die ten alle tijden goed staan.

Als uitbreiding op het vorige voorbeeld heb ik een extra rijrichting schakelaar toegevoegd (voor het andere station) en de wissels zijn voorzien van een dubbelpolige wisselmodule. Deze wisselmodule is in staat om de rijrichtingen naar de 0V te trekken. In dit scenario mag een sein pas in groen als beide rijrichting schakelaars goed staan en als beide wissels goed staan. Als een schakelaar of wissel verkeerd staat dan zijn de seinen in beide richtingen vergrendeld.

![alt text](https://raw.githubusercontent.com/bask185/SAS/master/images/img1-2.png "1-2")

Om in plaats van deze daadwerkelijke wisselbeveiliging toe te passen kan je, zoals ik eerder heb benoemd, magneetwissels met eindafschakeling mee laten schakelen op de rijrichting lijnen. De wissels kunnen dan fysiek nooit meer fout staan. Er komt wel een maar bij kijken. Beide rijrichting lijnen mogen onder geen omstandigheden tegelijkertijd naar 0V worden getrokken. Daarom mag er ook slechts 1 schakelaar zijn voor de rijrichting.

![alt text](https://raw.githubusercontent.com/bask185/SAS/master/images/img1-3.png "1-3")

Zoals in het vorige hoofdstuk ook is getoond, kan SAS ook hier gebruik maken van korte detectiestukjes. In dat geval is het nodig om ook de communicatie lijnen te trekken tussen de seinen.

![alt text](https://raw.githubusercontent.com/bask185/SAS/master/images/img1-4.png "1-4")
Zoals je ziet lijkt de beveiliging erg veel op dat van een dubbelspoor met als toevoeging twee rijrichtinglijnen en optionele wisselbeveilingen en wisselaansturing.

---
## Beveiliging van stations
Aanvankelijk wilde ik nog een stukje maken voor het bovenstaande voorbeeld met 
plaatjes

## Blokbeveiliging van een enkel sporig branchlijn met uni-directionele passeerstukken
plaatjes

## Beveiling van wisselstraten

## Beveiliging van stations