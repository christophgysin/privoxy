#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  This file should be the first thing included in every
 *                .c file.  (Before even system headers).  It contains 
 *                #define statements for various features.  It was
 *                introduced because the compile command line started
 *                getting ludicrously long with feature defines.
 *
 * Copyright   :  Written by and Copyright (C) 2001 the SourceForge
 *                IJBSWA team.  http://ijbswa.sourceforge.net
 *
 *                Based on the Internet Junkbuster originally written
 *                by and Copyright (C) 1997 Anonymous Coders and 
 *                Junkbusters Corporation.  http://www.junkbusters.com
 *
 *                This program is free software; you can redistribute it 
 *                and/or modify it under the terms of the GNU General
 *                Public License as published by the Free Software
 *                Foundation; either version 2 of the License, or (at
 *                your option) any later version.
 *
 *                This program is distributed in the hope that it will
 *                be useful, but WITHOUT ANY WARRANTY; without even the
 *                implied warranty of MERCHANTABILITY or FITNESS FOR A
 *                PARTICULAR PURPOSE.  See the GNU General Public
 *                License for more details.
 *
 *                The GNU General Public License should be included with
 *                this file.  If not, you can view it at
 *                http://www.gnu.org/copyleft/gpl.html
 *                or write to the Free Software Foundation, Inc., 59
 *                Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Revisions   :
 *    $Log$
 *    Revision 1.11  2001/09/13 01:24:07  david__schmidt
 *    A couple of minor tweaks:
 *    - Cleaned up warnings on OS/2 and OSX
 *    - Gave support to those needing <sys/malloc.h> instead of <malloc.h>
 *
 *    Revision 1.10  2001/07/30 22:08:36  jongfoster
 *    Tidying up #defines:
 *    - All feature #defines are now of the form FEATURE_xxx
 *    - Permanently turned off WIN_GUI_EDIT
 *    - Permanently turned on WEBDAV and SPLIT_PROXY_ARGS
 *
 *    Revision 1.9  2001/07/29 19:08:52  jongfoster
 *    Changing _CONFIG_H to CONFIG_H_INCLUDED.
 *    Also added protection against using a MinGW32 or CygWin version of
 *    config.h from within MS Visual C++
 *
 *    Revision 1.8  2001/07/29 17:09:17  jongfoster
 *    Major changes to build system in order to fix these bugs:
 *    - pthreads under Linux was broken - changed -lpthread to -pthread
 *    - Compiling in MinGW32 mode under CygWin now correctly detects
 *      which shared libraries are available
 *    - Solaris support (?) (Not tested under Solaris yet)
 *
 *    Revision 1.7  2001/07/25 22:53:59  jongfoster
 *    Will #error if pthreads is enabled under BeOs
 *
 *    Revision 1.6  2001/07/15 17:54:29  jongfoster
 *    Renaming #define STATIC to STATIC_PCRE
 *    Adding new #define FEATURE_PTHREAD that will be used to enable
 *    POSIX threads support.
 *
 *    Revision 1.5  2001/07/13 13:48:37  oes
 *     - (Fix:) Copied CODE_STATUS #define from config.h.in
 *     - split REGEX #define into REGEX_GNU and REGEX_PCRE
 *       and removed PCRE.
 *       (REGEX = REGEX_GNU || REGEX_PCRE per project.h)
 *     - Moved STATIC (for pcre) here from Makefile.in
 *     - Introduced STATIC_PCRS #define to allow for dynaimc linking with
 *       libpcrs
 *     - Removed PCRS #define, since pcrs is now needed for CGI anyway
 *
 *    Revision 1.4  2001/05/29 09:50:24  jongfoster
 *    Unified blocklist/image    U   3visioge  RE.
 *  * File fothatadst wild undes dcu3visiul, buo the intesuar chang *
 *  s ar(mgfoly) donert.
 *
 *    Alsr modifiee intcepectll havarires:
 *    Ware noe intcepeto a URLwaseangarning witoneow of the  allrni59
 *    prakexrie(  an*only*ix thesprakexri)es:
 *  *   *.  http:/.j.b/59
 *      *.  http://ijbs.sfge.nrg/conf/59
 *      *.  http://ijbs.s.sourceforge.nrg/conf/59
 *  - Nview intcepects "homnceima"   g too.  http:/.j.b/too.s Sei RE.
 *  -he Intesuar changee pe that intcepefied anfainsn reorreceimacts
 *    s are nsn p Plaied wit cat/imaRE.
 *  -he Intcepect is nohavSee thr optitoo.s  ana bigintreimaes orre59
 *    e to thcmplnt. (i.ilet/i-s  a-banGenbe uges th.h)
 *  -heTemptemeaiee sw-url-infoew intcepectle. W whictadwhy Iow need59
 *    e thab Moew intcepects r change-na typicsuaURLcta59
 *    "  http:/.j.b/e sw-url-info?url=//wwsomnsSuirs.c/banGen.gif"RE.
 *    T thpravaris mer chisromshoule nshavSew intcepeused th,ed a59
 *    r iplihteabeokew intcepeused okewbuo tkewbumshoulhavSen p Plai59
 *    rhat wit cat/imaRet)
 *
 *    Revision 3.4  2001/056 01:26:3:24  jongfoster
 *  Nview #defi,ff WIN_GUI_ED,to enabges ar(embryonic) ygWGW3N_G  reectRE.
 *    Thiw #definu ce nsl bsrnee fro.rg/confatu   eithe' is  pow i,ewb59
 *  d  on'bumsrk s ySE.  Ses featuen qu te # 425722ay
 *
 *    Revision :52  2001/05215 143:31.5  oes
ad
 *    E enablekefintrninbanGenwasye, zSenaeithed tnaURL59
 *      by AddinparitrionD than p Plato a e sounreabanGen59
 *    , zSadw wits ar"  Junkbust" gifte to thre_kefintkefies
ad
 *    E enablekefintrninWebB buasyer pviAddinanparitri59
 *      whick wiies a 1x1at/imaoes
ad
 *    A addee suppord fod PC_UNGREEDYll havarirte te pc,59
 *      whictadselrreshed by th( Ane sounread aneitheBefo59
 *    capital) l gethe'U'ed in thr optitdistngRE.
 *      Itecauges arqurraodifnwas toode g Frdyasyee daul RE.
 *     pp  addinan?ly tuuasackas tg Frdya(!)rt.
 *
 *    - addeang nee intcepectlt/i-s  a-banGen,dwhwhi59
 *    ,  auasackas ar"  Junkbust" gif. ygwithout/image  in 59
 *    MSIEly deteptite suppose, oifteinygift= 1se, on the
 *    URLctan'buorrogn zSdeaers ut/imaurl,edny mncHTML59
 *    exp Pnndatiatadsumen> instert.
 *
 *    - addeg nes featu,dwhwhiRE pmientd bloddind reto59
 *    scripnsn reorreers ane frninbackadnyocsuan reorre59
 *    e to ths bwsen.59
 *    T ths featuetadg/creeatiinalle compid,ew.ilete59
 *    u caoods denablew wit--s denab-fain-n reorrec,59
 *    plis wbumkbuaoodatecv lahed ba "fain-n reorrec"59
 *    d lind in thg/confis fi,lhas wbrmswnnyoingt etml
 *    s any oA crheswrrawas toods dp Pyahed be sw-r pxy-arga59
 *     (NrceBoy,ed a s ar7
#e ds wn 1 200yocseatiers a59
 *    d a s arfumPubning witg/confatule.rs and acconfig.t
 *    w her*nyw*or mormsrk d tnae the featu wbrelf :-(es
ad
 *    B becausa g Genblin reorrectatp Puitx waw needed f59
 *    e is,teinygift= 3is nocauges ars mnrt.
 *
 *    - MoveGIFs,ts anyeithee stbliHTTPin sptiethe tp Puia59
 *    e te projectes
ad
 *    M) anf minokexries
ad
 *    - Removesomn >400 CRers aga (Jsiul; yothrnallysrkrted
 *    dnyot! ;-et)
 *
 *    Revision n 1.152  2001/07/15:538:41.5  oes
*   Ineeasuaimuppory or version.9.3ee sour t Fres
ad
ad




































































/

@TOP@

/
ad
 V versionumPthe  - Majo(X._._et)
/
#d unf VERSION_MAJOR

/
ad
 V versionumPthe  - mino(_.X._et)
/
#d unf VERSION_MINOR

/
ad
 V versionumPthe  Pow io(_._.Xet)
/
#d unf VERSION_POINT

/
ad
 V versionumPth,tsersee frnit)
/
#d unf VERSION

/
ad
 S stis s of thc mo: "alpha", "b de"e, o"e snab"RE.
/
#d unf d CODE_STAT

/
ad
 ReguSol expn sersioothc thind foURLwle. Hyriallorromtemadd)rt.
.  If; eithes of tensemore defianeitnt, you caAnoocauhprao fiothc thirt.
.Don'bubyeithee tr chanes the) he! Uauhg/confatun> instert.
/
#d unf o REGEX_G
#d unf d REGEX_PCR
/
it 
 S shoulr pcsl bs stblrnallo buen> h> instead oc liubning wit  libpe?t 
 (  ThiThiy depmifiand bg/confatundep  addind on the avaiabvaianty t 
   libpets ancauper ef hesiners)T thRenu wadsgly,e, bur pcsw nes wbrt.
.Don'bubyeithee tr chanes the) he! Uauhg/confatun> instert.
/
#d unf o STATIC_PCR
/
it 
 S shoulr pwasebs stblrnallo buen> h> instead oc liubning wit  libps?t 
 (  ThiThiy depmifiand bg/confatundep  addind on the avaiabvaianty t 
   libpers ancauper ef hesinerst.
.Don'bubyeithee tr chanes the) he! Uauhg/confatun> instert.
/
#d unf d STATIC_PC

/
ad
 - Abwsae thcauhs otnaACLee tr corolnd a sete to thr pxynd bIPby An seRE.
/
#d unf e FEATURACL

/
ad
 - Abwsae thcauhs ojol s fiete tcapeatu cookariRE.
/
#d unf e FEATURCOOKIE_JAR

/
ad
   Isee f te wade con seianvia gzip,te tkewbuu ce nsl bkefintianeiicrgi59
 e thd PClorgexpwle. - Cmonlylhapp  adw witNetscapcsl bwseners59
   Thisoours wbus toode e con seiaRE.
/
#d unf e FEATURDENY_GZIP

/
ad
 Locsuallorreorrecd reto scripn-n reorreoURLwE.
/
#d unf e FEATURFAST_RI_ERECTS

/
ad
 Bypasetkefintrnind fo1reimaeonlyE.
/
#d unf e FEATURFORCE_LOAD

/
ad
 - Abwtd bloddint usint/imaotserwe a serHTMLrt.
.  I; yodole ns
#defins thee tken evet thingntd bloSdeaerHTMLrt.
59
  (NrnD thas thethen qu orieeft, yowrraus tcauhe FEATURIMAGE_DETECT_MSIERE.
/
#d unf e FEATURIMAGE_BLOCKING

/
ad
 D detent/imaen qu teotsutoothblrnalld foMSIER :  wilfd a sackas 59
 yeithet/ima-y detersiooewites (i.ile"+t/ima"RE pmisersi)nd foyeith59
 l bwsenert.
 *
 Y yomkbuaa Als
#define FEATURIMAGE_BLOCKINGus tcauh  this featurt.
 *
   Iy detecae theo allrniem headreiireaers ut/imaen qu tees:
59
 Uaur-Ag Gt: - z wia/4.0 (e cothbnab;oMSIE 5.5; ygWdbwsaNT 5.0.h)
 A a pt: * /s
ad
ad
97 aneitheo allrnieserseHTMLen qu tees:
59
 Uaur-Ag Gt: - z wia/4.0 (e cothbnab;oMSIE 5.5; ygWdbwsaNT 5.0.h)
 A a pt: t/ima/gif,ew/ima/x-xbitmap,tw/ima/jpeg,tw/ima/pjpeg,t* /s
ad
ad
97 annoon,lhavSn'bug(Not thasackd wes -heEethebebning erdrt.
 *
 Ks nnnyimieseatie:it 
 1).  I, yoon sebe ifn-n lotead.rstd blorieHTMLeeimaul; yo  g59
 *  o the/imae"d blori"REimaule nso thHTMLe"d blori"REimart.
.2) Osincs ut/imae"d blori"REimalhas beokesume,nvielrnite it 
 *  > h>t' i nnnl bwsenng Wdbw *e shou
 l dying uo thHTML59
 *  "d blori"REimaul, buwbud  on'bR :Y yow nete tc Clder th59
 *  l bwsenncac the tgrneo thHTMLer versios agart.
 *
 T tauhyimieseatiesemoreuthe teEemaoddin sitiee  umencho Lia59
 ab  bu  whic"A a pt:"em headroo.s  aRE.
/
#d unf e FEATURIMAGE_DETECT_MSIE

/
ad
 K wiieJe aScripnspopups -hg Wdbw. hoiulonunlote,totcRE.
/
#d unf e FEATURKILL_POPUPS

/
ad
 Uauh  POSIX thread> instead onndavSee threaRE.
/
#d unf e FEATURE_PTHRE

/
ad
 E enabadstndast chisuntersiRE.
/
#d unf e FEATURd STASTATS

/
ad
 - Abwet JuBkbustes tood"s denabl"ee pwbuthejkbuaale fotlle n-d bloddi59
 e n-a Anonizddinp pxyR :  ThiThie usefeeft, y'morEntddine to a seta59
 l bloriectll brokesSui -hjkbuar chanes  bsrnettind in thg/confis fi,59
 ynbe ueo th chdya"D denab"ooenuhr optitd in thygWdbwsaN_GRE.
/
#d unf e FEATURTOGGLE

/
ad
 - Abwsae thcauhs otrkbuas fieRE.
/
#d unf e FEATURTRUST

/
ad
 D defiand.rr SolarionlyR :m Masae thd systed librariee thre safurt.
/
#d unf _RIENTARRA

/
ad
 D defiand.rr SolarionlyR :ygwithou  th,emanooimupporrausuntersiies are n59
 e defiand in thn system headert.
/
#d unf __EXTENSIONS__

/
ad
 D defianalnyweRE.
or XME:.Don'buks now that id  o ynbwhy wcsw ne wbrt.
.(on sumena beomnt thine tdolw witMul iT threthi?et)
/
#d unf __MT__

@BOTTOM@

/
ad
 D defianalnyweRE.
or XME:.Don'buks now that id  o ynbwhy wcsw ne wbrt.
.(on sumena beomnt thine tdolw witANSI S sounreaC?et)
/
#
#ifnde_DE_DC__
w #defin_DE_DC__ 1
#  adf /
i unf _DE_DC__ 
/

/
ad
 N nete tsrneg u  thi
#definonlyld foe thdX threadd libryld fE.
oygWGW,ts availabee fro  http:e sours.ori thrs.c/f pthrea-g W32/59
/
#df e defia(e FEATURE_PTHRE) && e defia(_f W32)
w #defin_DCLEANUP_C
#  adf /
ie defia(e FEATURE_PTHRE) && e defia(_f W32)9
/

/
ad
 BEOSud  ole nsa/currea be suppor  POSIX threas59
   Thi*e shou
 lore drreshed b.rg/confatu,e, bul g' isebsaturt.
/
#df e defia(e FEATURE_PTHRE) && e defia(__BEOS__etl #erroBEOSud  ole nse suppor-lpthrea- emp Ba runo.rg/confatu s aga w wit"--s denab--lpthre"

#  adf /
ie defia(e FEATURE_PTHRE) && e defia(__BEOS__e9
/


/
ad
 Ig' ito toastly to aidumesualle ueaer Cg W ynba MinGW3n version om configE.
od undeVl C,ts any itVisuallgavSs manoog erd  #erromnssimaoR :L g' imaoo59
 e th #erromnssimaood unde sounnab,e,yndavaihin  bus nrt.
/
#
#e d _MSC_VERtl #erroceFin MSl C,temp Ba e uem config.g W ynbm config.g W32e threaRg WR :Y you catVisualldol  thid beelrrethine ar"B bui", "- Cle"ooenuhr opti.
#  adf /
ie d _MSC_VER9
/

/
ad
 ceFio thosn systwaw nething <sys/malloc.h> instead of <malloc...rt.
/
#d unf NEED_SYS_MALLOC

#  adf /
io CONFIG_H_INCLUD9
/
t
