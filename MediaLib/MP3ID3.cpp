
#include "MP3ID3.h"

#include <cstring>

//定义 MP3 文件的音乐类型

static int m_nMP3ID3v2GenreCount = 148;

static char* m_arrMP3ID3v2Genre[] = {

    "Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge","Hip-Hop",

    "Jazz","Meta","New Age","Oldies","Other","Pop","R&B","Rap","Reggae","Rock",

    "Techno","Industria","Alternative","Ska","Death Meta","Pranks","Soundtrack",

    "Euro-Techno","Ambient","Trip Hop","Voca","Jazz Funk","Fusion","Trance",

    "Classica","Instrumenta","Acid","House","Game","Sound Clip","Gospe","Noise",

    "Alternative Rock","Bass","Sou","Punk","Space","Meditative","Instrumental Pop",

    "Instrumental Rock","Ethnic","Gothic","Darkwave","Techno-Industria","Electronic",

    "Pop Folk","Eurodance","Dream","Southern Rock","Comedy","Cult","Gangsta","Top 40",

    "Christian Rap","Pop Funk","Jungle","Native American","Cabaret","New Wave",

    "Psychadelic","Rave","ShowTunes","Trailer","Lo-Fi","Triba","Acid Punk","Acid Jazz",

    "Polka","Retro","Musica","Rock & Rol","Hard Rock","Folk","Folk Rock",

    "National Folk","Swing","Fast Fusion","Bebob","Latin","Reviva","Celtic",

    "Bluegrass","Avantgarde","Gothic Rock","Progressive Rock","Psychedelic Rock",

    "Symphonic Rock","Slow Rock","Big Band","Chorus","Easy Listening","Acoustic",

    "Humour","Speech","Chanson","Opera","Chamber Music","Sonata","Symphony","Booty Bass",

    "Primus","Porn Groove","Satire","Slow Jam","Club","Tango","Samba","Folklore","Ballad",

    "Power Ballad","Rhytmic Sou","Freestyle","Duet","Punk Rock","Drum Solo","A Capella",

    "Euro House","Dance Hal","Goa","Drum & Bass","Club House","Hardcore","Terror",

    "Indie","BritPop","Negerpunk","Polsk Punk","Beat","Christian Gangsta Rap",

    "Heavy Meta","Black Meta","Crossover","Contemporary Christian","Christian Rock",

    "Merengue","Salsa","Trash Meta","Anime","JPop","SynthPop"

};

//定义一些使用到的结构体

//define frame id (see in ID3v2.3)

//#define ID3V2_FRAMEID_TITLE     "TIT2"    //Title

//#define ID3V2_FRAMEID_ARTIST    "TPE1"    //Artist

//#define ID3V2_FRAMEID_ALBUM     "TALB"    //Album

//#define ID3V2_FRAMEID_TRACK     "TRCK"    //Track

//#define ID3V2_FRAMEID_YEAR      "TYER"    //Year

//#define ID3V2_FRAMEID_COMMENT   "COMM"    //Comment

//#define ID3V2_FRAMEID_GENRE     "TCON"    //Genre

//#define ID3V2_FRAMEID_ENCODEBY  "TENC"    //Encode By(编码方式)

//#define ID3V2_FRAMEID_COPYRIGHT "TCOP"    //Copyright

//#define  ID3V2_FRAMEID_URL       "WXXX"    //URL

static char* m_pszID3V2FrameId[] =
{
  "????", /* ID3FID_NOFRAME = 0,        */ /**< No known frame */

  "AENC", /* ID3FID_AUDIOCRYPTO,        */ /**< Audio encryption */

  "APIC", /* ID3FID_PICTURE,            */ /**< Attached picture */

  "ASPI", /* ID3FID_AUDIOSEEKPOINT,     */ /**< Audio seek point index */

  "COMM", /* ID3FID_COMMENT,            */ /**< Comments */

  "COMR", /* ID3FID_COMMERCIAL,         */ /**< Commercial frame */

  "ENCR", /* ID3FID_CRYPTOREG,          */ /**< Encryption method registration */

  "EQU2", /* ID3FID_EQUALIZATION2,      */ /**< Equalisation (2) */

  "EQUA", /* ID3FID_EQUALIZATION,       */ /**< Equalization */

  "ETCO", /* ID3FID_EVENTTIMING,        */ /**< Event timing codes */

  "GEOB", /* ID3FID_GENERALOBJECT,      */ /**< General encapsulated object */

  "GRID", /* ID3FID_GROUPINGREG,        */ /**< Group identification registration */

  "IPLS", /* ID3FID_INVOLVEDPEOPLE,     */ /**< Involved people list */

  "LINK", /* ID3FID_LINKEDINFO,         */ /**< Linked information */

  "MCDI", /* ID3FID_CDID,               */ /**< Music CD identifier */

  "MLLT", /* ID3FID_MPEGLOOKUP,         */ /**< MPEG location lookup table */

  "OWNE", /* ID3FID_OWNERSHIP,          */ /**< Ownership frame */

  "PRIV", /* ID3FID_PRIVATE,            */ /**< Private frame */

  "PCNT", /* ID3FID_PLAYCOUNTER,        */ /**< Play counter */

  "POPM", /* ID3FID_POPULARIMETER,      */ /**< Popularimeter */

  "POSS", /* ID3FID_POSITIONSYNC,       */ /**< Position synchronisation frame */

  "RBUF", /* ID3FID_BUFFERSIZE,         */ /**< Recommended buffer size */

  "RVA2", /* ID3FID_VOLUMEADJ2,         */ /**< Relative volume adjustment (2) */

  "RVAD", /* ID3FID_VOLUMEADJ,          */ /**< Relative volume adjustment */

  "RVRB", /* ID3FID_REVERB,             */ /**< Reverb */

  "SEEK", /* ID3FID_SEEKFRAME,          */ /**< Seek frame */

  "SIGN", /* ID3FID_SIGNATURE,          */ /**< Signature frame */

  "SYLT", /* ID3FID_SYNCEDLYRICS,       */ /**< Synchronized lyric/text */

  "SYTC", /* ID3FID_SYNCEDTEMPO,        */ /**< Synchronized tempo codes */

  "TALB", /* ID3FID_ALBUM,              */ /**< Album/Movie/Show title */

  "TBPM", /* ID3FID_BPM,                */ /**< BPM (beats per minute) */

  "TCOM", /* ID3FID_COMPOSER,           */ /**< Composer */

  "TCON", /* ID3FID_CONTENTTYPE,        */ /**< Content type */

  "TCOP", /* ID3FID_COPYRIGHT,          */ /**< Copyright message */

  "TDAT", /* ID3FID_DATE,               */ /**< Date */

  "TDEN", /* ID3FID_ENCODINGTIME,       */ /**< Encoding time */

  "TDLY", /* ID3FID_PLAYLISTDELAY,      */ /**< Playlist delay */

  "TDOR", /* ID3FID_ORIGRELEASETIME,    */ /**< Original release time */

  "TDRC", /* ID3FID_RECORDINGTIME,      */ /**< Recording time */

  "TDR", /* ID3FID_RELEASETIME,        */ /**< Release time */

  "TDTG", /* ID3FID_TAGGINGTIME,        */ /**< Tagging time */

  "TIP", /* ID3FID_INVOLVEDPEOPLE2,    */ /**< Involved people list */

  "TENC", /* ID3FID_ENCODEDBY,          */ /**< Encoded by */

  "TEXT", /* ID3FID_LYRICIST,           */ /**< Lyricist/Text writer */

  "TFLT", /* ID3FID_FILETYPE,           */ /**< File type */

  "TIME", /* ID3FID_TIME,               */ /**< Time */

  "TIT1", /* ID3FID_CONTENTGROUP,       */ /**< Content group description */

  "TIT2", /* ID3FID_TITLE,              */ /**< Title/songname/content description */

  "TIT3", /* ID3FID_SUBTITLE,           */ /**< Subtitle/Description refinement */

  "TKEY", /* ID3FID_INITIALKEY,         */ /**< Initial key */

  "TLAN", /* ID3FID_LANGUAGE,           */ /**< Language(s) */

  "TLEN", /* ID3FID_SONGLEN,            */ /**< Length */

  "TMC", /* ID3FID_MUSICIANCREDITLIST, */ /**< Musician credits list */

  "TMED", /* ID3FID_MEDIATYPE,          */ /**< Media type */

  "TMOO", /* ID3FID_MOOD,               */ /**< Mood */

  "TOA", /* ID3FID_ORIGALBUM,          */ /**< Original album/movie/show title */

  "TOFN", /* ID3FID_ORIGFILENAME,       */ /**< Original filename */

  "TOLY", /* ID3FID_ORIGLYRICIST,       */ /**< Original lyricist(s)/text writer(s) */

  "TOPE", /* ID3FID_ORIGARTIST,         */ /**< Original artist(s)/performer(s) */

  "TORY", /* ID3FID_ORIGYEAR,           */ /**< Original release year */

  "TOWN", /* ID3FID_FILEOWNER,          */ /**< File owner/licensee */

  "TPE1", /* ID3FID_LEADARTIST,         */ /**< Lead performer(s)/Soloist(s) */

  "TPE2", /* ID3FID_BAND,               */ /**< Band/orchestra/accompaniment */

  "TPE3", /* ID3FID_CONDUCTOR,          */ /**< Conductor/performer refinement */

  "TPE4", /* ID3FID_MIXARTIST,          */ /**< Interpreted, remixed, or otherwise modified by */

  "TPOS", /* ID3FID_PARTINSET,          */ /**< Part of a set */

  "TPRO", /* ID3FID_PRODUCEDNOTICE,     */ /**< Produced notice */

  "TPUB", /* ID3FID_PUBLISHER,          */ /**< Publisher */

  "TRCK", /* ID3FID_TRACKNUM,           */ /**< Track number/Position in set */

  "TRDA", /* ID3FID_RECORDINGDATES,     */ /**< Recording dates */

  "TRSN", /* ID3FID_NETRADIOSTATION,    */ /**< Internet radio station name */

  "TRSO", /* ID3FID_NETRADIOOWNER,      */ /**< Internet radio station owner */

  "TSIZ", /* ID3FID_SIZE,               */ /**< Size */

  "TSOA", /* ID3FID_ALBUMSORTORDER,     */ /**< Album sort order */

  "TSOP", /* ID3FID_PERFORMERSORTORDER, */ /**< Performer sort order */

  "TSOT", /* ID3FID_TITLESORTORDER,     */ /**< Title sort order */

  "TSRC", /* ID3FID_ISRC,               */ /**< ISRC (international standard recording code) */

  "TSSE", /* ID3FID_ENCODERSETTINGS,    */ /**< Software/Hardware and settings used for encoding */

  "TSST", /* ID3FID_SETSUBTITLE,        */ /**< Set subtitle */

  "TXXX", /* ID3FID_USERTEXT,           */ /**< User defined text information */

  "TYER", /* ID3FID_YEAR,               */ /**< Year */

  "UFID", /* ID3FID_UNIQUEFILEID,       */ /**< Unique file identifier */

  "USER", /* ID3FID_TERMSOFUSE,         */ /**< Terms of use */

  "USLT", /* ID3FID_UNSYNCEDLYRICS,     */ /**< Unsynchronized lyric/text transcription */

  "WCOM", /* ID3FID_WWWCOMMERCIALINFO,  */ /**< Commercial information */

  "WCOP", /* ID3FID_WWWCOPYRIGHT,       */ /**< Copyright/Legal infromation */

  "WOAF", /* ID3FID_WWWAUDIOFILE,       */ /**< Official audio file webpage */

  "WOAR", /* ID3FID_WWWARTIST,          */ /**< Official artist/performer webpage */

  "WOAS", /* ID3FID_WWWAUDIOSOURCE,     */ /**< Official audio source webpage */

  "WORS", /* ID3FID_WWWRADIOPAGE,       */ /**< Official internet radio station homepage */

  "WPAY", /* ID3FID_WWWPAYMENT,         */ /**< Payment */

  "WPUB", /* ID3FID_WWWPUBLISHER,       */ /**< Official publisher webpage */

  "WXXX", /* ID3FID_WWWUSER,            */ /**< User defined URL link */

  "    ", /* ID3FID_METACRYPTO,         */ /**< Encrypted meta frame (id3v2.2.x) */

  "    ", /* ID3FID_METACOMPRESSION,    */ /**< Compressed meta frame (id3v2.2.1) */

  ">>>>"  /* ID3FID_LASTFRAMEID         */ /**< Last field placeholder */

};

//

typedef struct tagID3v2FrameHeader
{
    char   ifh_id[4];          //标志位

    UCHAR   ifh_pszlength[4];   //接下来那个 TAG 的长度

    UCHAR   ifh_flags[2];       //标志。在当前版本中不使用(2005-10-24)

  char   ifh_pad;            //填充。必须为 \0

  //整个 Frame 的大小为 ifh_pszlength(需要进行计算转化)+=11

  DWORD   ifh_info_length;    //ifh_pszlength转化过来的长度

  DWORD   ifh_size;           //Frame 的大小

  DWORD   ifh_pos;            //这个 Frame 在文件中的位置(相对于文件起始)

}ID3v2FrameHeader;

 

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//

//                  下面开始读取 MP3ID3V1 信息

//

//////////////////////////////////////////////////////////////////////////

 

/*********************************************************************

  函数声明: int AudioInfo_GetMP3ID3V1(HFILE hAudioFile, MP3ID3V1INFO *pMp3ID3V1Info)

  参    数:

          IN: HFILE hMP3File: 需要读取信息的 MP3 文件

              MP3ID3V1INFO *pMp3ID3V1Info: 读取了 MP3  信息的返回结构体缓冲

         OUT:

         I/O:

    返回值: 成功返回>=0，失败返回<0。这时可以使用函数 Audio_GetErrorString 来获取失败信息

            如果该 MP3 文件中存在 ID3V1，那么 bHasTag == TRUE

  功能描述: 读取 MP3 中的 ID3V1 信息

  引    用:

*********************************************************************/

int AudioInfo_GetMP3ID3V1(FILE *lpMP3File, MP3ID3V1INFO& MP3ID3V1Info)
{
  DWORD dwFileRead = 0;

  int i = 0;

  //判断该文件的长度是否的>128字节
  
  //SetFilePointer((HANDLE)hMP3File, 0x00, 0, FILE_BEGIN);
  //dwFileRead = SetFilePointer((HANDLE)hMP3File, 0x00, 0, FILE_END);
  //if(dwFileRead<=128)
  //{
  //  return -1;
  //}

  //SetFilePointer((HANDLE)hMP3File, -128, 0, FILE_END);
  fseek(lpMP3File, -128, SEEK_END);
  dwFileRead = fread(&MP3ID3V1Info, 1, 128, lpMP3File);
  //一次性读取这128个字节
  //ReadFile((HANDLE)hMP3File, pMP3ID3V1Info, 128, &dwFileRead, NULL);
  if(dwFileRead != 128)
  {
    return -1;
  }

  //判断是否拥有 ID3V1 信息
  if(MP3ID3V1Info.Identify[0]!='T'
    || MP3ID3V1Info.Identify[1]!='A'
    || MP3ID3V1Info.Identify[2]!='G')
  {
    MP3ID3V1Info.bHasTag = FALSE;

	return -1;
  }

  //判断 TAG 信息是否有效。就是判断剩余的 125 个字节是否全部是空格或是 \0
  for(i=3;i<128;i++)
  {
    if(((UCHAR*)&MP3ID3V1Info)[i] != 0x20 || ((UCHAR*)&MP3ID3V1Info)[i] != 0x00)
    {
      MP3ID3V1Info.bHasTag = TRUE;

      break;
    }
  }

  if(!MP3ID3V1Info.bHasTag)
  {
    return -1;
  }

  //接下来分析。

  //}

  MP3ID3V1Info.Genre = MP3ID3V1Info.CommentFlag;

  MP3ID3V1Info.CommentFlag = MP3ID3V1Info.Comment[28];

  MP3ID3V1Info.Track = MP3ID3V1Info.Comment[29];

  //有些软件会把 Title, Artist 等信息最后为 0x20 而不是\0

  //下面取出末尾的 0x20

  //去除 Title
  MP3ID3V1Info.Title[29] = '\0';

  for(i=28;
      i>=0&&(MP3ID3V1Info.Title[i]==0x20 || MP3ID3V1Info.Title[i]==0x00);
      i--)
  {
    MP3ID3V1Info.Title[i] = '\0';
  }

  //去除 Artist
  MP3ID3V1Info.Artist[29] = '\0';

  for(i=28;
      i>=0&&(MP3ID3V1Info.Artist[i]==0x20 || MP3ID3V1Info.Artist[i]==0x00);
      i--)
  {
    MP3ID3V1Info.Artist[i] = '\0';
  }

  //去除 Album
  MP3ID3V1Info.Album[29] = '\0';

  for(i=28; i>=0&&(MP3ID3V1Info.Album[i]==0x20 || MP3ID3V1Info.Album[i]==0x00); i--)
  {
    MP3ID3V1Info.Album[i] = '\0';
  }

  //保存 音乐的类型
  if (MP3ID3V1Info.Genre >= 0 && MP3ID3V1Info.Genre < m_nMP3ID3v2GenreCount)
  {
	  strcpy_s(MP3ID3V1Info.pszGenre, sizeof MP3ID3V1Info.pszGenre, m_arrMP3ID3v2Genre[MP3ID3V1Info.Genre]);
  }

  return 0;

}

//////////////////////////////////////////////////////////////////////////

//

//                  下面开始读取 MP3 ID3V2 信息

//

//////////////////////////////////////////////////////////////////////////

 

/*********************************************************************

  函数声明:

  参    数:

          IN:

         OUT:

         I/O:

    返回值:

  功能描述: 判断这个 MP3 中是否包含 ID3V2 信息。如果存在返回TRUE，否则不存在

            并且获取这个ID3V2 信息的长度。

  引    用:

*********************************************************************/

static BOOL AudioInfo_CheckMP3ID3V2(FILE *lpMP3File, MP3ID3V2INFO& MP3ID3V2Info)
{
  DWORD dwFileRead = 0;

  int i = 0;

  DWORD dwID3V2Length = 0;

  MP3ID3V2Info.bHasTag = FALSE;

  //读取 MP3 文件的最前面 3 个字节，判断是否是 "ID3"
  //ReadFile((HANDLE)hMP3File, (VOID*)pMP3ID3V2Info, 10, &dwFileRead, NULL);
  dwFileRead = fread((VOID*)&MP3ID3V2Info, 1, 10, lpMP3File);
  if(dwFileRead != 10)
  {
    return FALSE;
  }

  if( MP3ID3V2Info.Identify[0]=='I'
    &&MP3ID3V2Info.Identify[1]=='D'
    &&MP3ID3V2Info.Identify[2]=='3')
  {
	  MP3ID3V2Info.bHasTag = TRUE;
  }
  else
  {
	  return FALSE;
  }

  //判断是否含有扩展头部和扩展尾部

  if(MP3ID3V2Info.HeaderFlags&0x64)
  {
    MP3ID3V2Info.bHasExtHeader = TRUE;
  }

  if(MP3ID3V2Info.HeaderFlags&0x10)
  {
    MP3ID3V2Info.bHasExtHeader = TRUE;
  }

  //获取 ID3V1 信息的长度

    for(i=0;i<3;i++)
  {
    dwID3V2Length = dwID3V2Length + MP3ID3V2Info.HeaderSize[i];

    dwID3V2Length = dwID3V2Length * 0x80;
  }

  dwID3V2Length = dwID3V2Length + MP3ID3V2Info.HeaderSize[3];

  MP3ID3V2Info.dwInfoSize = dwID3V2Length;

  return TRUE;

}

 

/*********************************************************************

  函数声明:

  参    数:

          IN:

         OUT:

         I/O:

    返回值: 返回 Frame 的个数

  功能描述: 获取 ID3V2 中的 FrameHeader

  引    用:

*********************************************************************/

static UINT AudioInfo_GetFrameHeader(FILE *lpMP3File,  MP3ID3V2INFO& MP3ID3V2Info)
{
  DWORD dwFileRead = 0;
  UINT  uFrameCount = 0;
  ID3v2FrameHeader tFrmHeader = {0};
  UINT uFrameID = ID3FID_NOFRAME;
  int i = 0;

  for(DWORD dwFilePos=10; dwFilePos<MP3ID3V2Info.dwInfoSize;)
  {
	 fseek(lpMP3File, dwFilePos, 0);

    //SetFilePointer((HANDLE)hMP3File, dwFilePos, 0, FILE_BEGIN);

    //ReadFile((HANDLE)hMP3File, (VOID*)&tFrmHeader, 11, &dwFileRead, NULL);
    //ASSERT(dwFileRead == 11);

	 dwFileRead = fread((VOID*)&tFrmHeader, 1, 11, lpMP3File);
	 if (11 != dwFileRead)
	 {
		 return FALSE;
	 }

    dwFilePos += dwFileRead;

    if(tFrmHeader.ifh_id[0] != 0x00)
    {
      uFrameCount++;      //Frame 个数 +1

      tFrmHeader.ifh_info_length = 0;

      for(i=0;i<3;i++)
      {
        tFrmHeader.ifh_info_length = tFrmHeader.ifh_info_length+tFrmHeader.ifh_pszlength[i];

        tFrmHeader.ifh_info_length = tFrmHeader.ifh_info_length*0x80;
      }

      tFrmHeader.ifh_info_length = tFrmHeader.ifh_info_length + tFrmHeader.ifh_pszlength[3];
	  //上面已经计算出了偏移量及长度和求出了 FrameID，下面从文件中读取这个信息
	  if (0 == tFrmHeader.ifh_info_length)
	  {
		  return 0;
	  }

	  uFrameID = ID3FID_LASTFRAMEID;
	  for (UINT uFID = 0; uFID < ID3FID_MAX_COUNT; uFID++)
	  {
		  if (strncmp(tFrmHeader.ifh_id, m_pszID3V2FrameId[uFID], 4) == 0)
		  {
			  uFrameID = uFID;
			  break;
		  }
	  }
	  
	 std::string& strTagInfo = MP3ID3V2Info.ppszTagInfo[uFrameID];
	 strTagInfo.resize(tFrmHeader.ifh_info_length);// = (LPSTR)malloc(tFrmHeader.ifh_info_length);

      //memset(MP3ID3V2Info.ppszTagInfo[uFrameID].front(), 0x00, tFrmHeader.ifh_info_length);

      //ReadFile((HANDLE)hMP3File, MP3ID3V2Info.ppszTagInfo[uFrameID], tFrmHeader.ifh_info_length-1, &dwFileRead, NULL);
	  dwFileRead = fread((char*)strTagInfo.data(), 1, tFrmHeader.ifh_info_length - 1, lpMP3File);

      dwFilePos += dwFileRead;
    }
  }

  return uFrameCount;
}

 

/*********************************************************************

  函数声明: VOID AudioInfo_MP3ID3V2Free(MP3ID3V2INFO *pMP3ID3V2Info)

  参    数:

          IN: MP3ID3V2INFO *pMP3ID3V2Info: 需要清空的结构体

         OUT:

         I/O:

    返回值:

  功能描述: 清空 MP3ID3V2INFO 中的信息，回收内存

  引    用:

*********************************************************************/
//
//VOID AudioInfo_MP3ID3V2Free(MP3ID3V2INFO *pMP3ID3V2Info)
//{
//  UINT uFrameID = 0;
//
//  ASSERT(pMP3ID3V2Info!=NULL);
//
//  for(uFrameID=0; uFrameID<ID3FID_MAX_COUNT;uFrameID++)
//  {
//    if(MP3ID3V2Info.ppszTagInfo[uFrameID]!=NULL)
//    {
//      free(MP3ID3V2Info.ppszTagInfo[uFrameID]);
//
//      MP3ID3V2Info.ppszTagInfo[uFrameID] = NULL;
//    }
//  }
//}

/*********************************************************************

  函数声明: int AudioInfo_GetMP3ID3V2(HFILE hMP3File, MP3ID3V2INFO *pMP3ID3V2Info)

  参    数:

          IN: HFILE hMP3File: 需要读取信息的 MP3 文件

              MP3ID3V2INFO *pMp3ID3V2Info: 读取了 MP3  信息的返回结构体缓冲

         OUT:

         I/O:

    返回值: 成功返回>=0，失败返回<0。这时可以使用函数 Audio_GetErrorString 来获取失败信息

            如果该 MP3 文件中存在 ID3V2，那么 bHasTag == TRUE

  功能描述: 读取 MP3 中的 ID3V2 信息

      注意: 使用ID3V2信息完毕后，请调用函数 AudioInfo_MP3ID3V2Free(MP3ID3V2INFO *pMP3ID3V2Info)

  引    用:

*********************************************************************/

int AudioInfo_GetMP3ID3V2(FILE *lpMP3File, MP3ID3V2INFO& MP3ID3V2Info)
{
  //SetFilePointer((HANDLE)hMP3File, 0x00, 0, FILE_BEGIN);
  fseek(lpMP3File, 0, 0);

  //检查文件中中是否有 ID3V2 Tag 信息
  if(!AudioInfo_CheckMP3ID3V2(lpMP3File, MP3ID3V2Info))
  {
		//ASSERT(MP3ID3V2Info.bHasTag == FALSE);
		
		return -1;
  }

  //检查 MP3 文件是否含有 ID3V2 信息完毕。到这一步确定已经包含 ID3V2 信息

  //并且已经得到了 ID3V2 信息的长度
   
  //在本版本中，仅仅支持 ID3V2 信息的 V3 版本或以下

  //这个版本中，不支持 ExtHeader，所以下面判断是否存在 ExtHeader

  if(MP3ID3V2Info.bHasExtHeader)
  {
    return -1;
  }

  //获取 ID3V2 FrameHeader

  AudioInfo_GetFrameHeader(lpMP3File, MP3ID3V2Info);

  return 0;
}
