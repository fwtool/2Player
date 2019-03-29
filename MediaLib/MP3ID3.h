#include <string>

typedef unsigned char UCHAR;

typedef int BOOL;

typedef unsigned int UINT;

#define TRUE 1
#define FALSE 0

typedef unsigned long DWORD;

#define VOID void

//定义读取 MP3 ID3V1 所需要的结构体

//MP3 ID3V1 信息保存到MP3文件的最后128个字节中

typedef struct tag_MP3ID3V1INFO      //MP3信息的结构ShitMP3v1.07
{
  char   Identify[3];     //TAG三个字母。这里可以用来鉴别是不是文件信息内容

  char   Title[30];       //歌曲名，30个字节

  char   Artist[30];      //歌手名，30个字节

  char   Album[30];       //所属唱片，30个字节

  char   Year[4];         //年，4个字节

  char   Comment[28];     //注释，一般为28个字节（也可能30个字节，这时候占用下面的2个字节）

  UCHAR   CommentFlag;     //保留位1，注释长度标志位。如果是 0x00 表明注释长度为28，否则为30

  UCHAR   Track;           //保留位2，那个“第几首”，是个整数

  UCHAR   Genre;           //保留位3，歌曲风格，在0－148之间。例如Pop，General...

  char   pszGenre[30];    //保存了音乐的类型。

 //

 BOOL   bHasTag;         //这个 MP3 是否存在 ID3V1 TAG信息

} MP3ID3V1INFO, *PMP3ID3V1INFO;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//MP3 ID3V2 FrameID

enum ID3_FrameID{

  /* ???? */ ID3FID_NOFRAME = 0,       /**< No known frame */

  /* AENC */ ID3FID_AUDIOCRYPTO,       /**< Audio encryption */

  /* APIC */ ID3FID_PICTURE,           /**< Attached picture */

  /* ASPI */ ID3FID_AUDIOSEEKPOINT,    /**< Audio seek point index */

  /* COMM */ ID3FID_COMMENT,           /**< Comments */

  /* COMR */ ID3FID_COMMERCIAL,        /**< Commercial frame */

  /* ENCR */ ID3FID_CRYPTOREG,         /**< Encryption method registration */

  /* EQU2 */ ID3FID_EQUALIZATION2,     /**< Equalisation (2) */

  /* EQUA */ ID3FID_EQUALIZATION,      /**< Equalization */

  /* ETCO */ ID3FID_EVENTTIMING,       /**< Event timing codes */

  /* GEOB */ ID3FID_GENERALOBJECT,     /**< General encapsulated object */

  /* GRID */ ID3FID_GROUPINGREG,       /**< Group identification registration */

  /* IPLS */ ID3FID_INVOLVEDPEOPLE,    /**< Involved people list */

  /* LINK */ ID3FID_LINKEDINFO,        /**< Linked information */

  /* MCDI */ ID3FID_CDID,              /**< Music CD identifier */

  /* MLLT */ ID3FID_MPEGLOOKUP,        /**< MPEG location lookup table */

  /* OWNE */ ID3FID_OWNERSHIP,         /**< Ownership frame */

  /* PRIV */ ID3FID_PRIVATE,           /**< Private frame */

  /* PCNT */ ID3FID_PLAYCOUNTER,       /**< Play counter */

  /* POPM */ ID3FID_POPULARIMETER,     /**< Popularimeter */

  /* POSS */ ID3FID_POSITIONSYNC,      /**< Position synchronisation frame */

  /* RBUF */ ID3FID_BUFFERSIZE,        /**< Recommended buffer size */

  /* RVA2 */ ID3FID_VOLUMEADJ2,        /**< Relative volume adjustment (2) */

  /* RVAD */ ID3FID_VOLUMEADJ,         /**< Relative volume adjustment */

  /* RVRB */ ID3FID_REVERB,            /**< Reverb */

  /* SEEK */ ID3FID_SEEKFRAME,         /**< Seek frame */

  /* SIGN */ ID3FID_SIGNATURE,         /**< Signature frame */

  /* SYLT */ ID3FID_SYNCEDLYRICS,      /**< Synchronized lyric/text */

  /* SYTC */ ID3FID_SYNCEDTEMPO,       /**< Synchronized tempo codes */

  /* TALB */ ID3FID_ALBUM,             /**< Album/Movie/Show title */

  /* TBPM */ ID3FID_BPM,               /**< BPM (beats per minute) */

  /* TCOM */ ID3FID_COMPOSER,          /**< Composer */

  /* TCON */ ID3FID_CONTENTTYPE,       /**< Content type */

  /* TCOP */ ID3FID_COPYRIGHT,         /**< Copyright message */

  /* TDAT */ ID3FID_DATE,              /**< Date */

  /* TDEN */ ID3FID_ENCODINGTIME,      /**< Encoding time */

  /* TDLY */ ID3FID_PLAYLISTDELAY,     /**< Playlist delay */

  /* TDOR */ ID3FID_ORIGRELEASETIME,   /**< Original release time */

  /* TDRC */ ID3FID_RECORDINGTIME,     /**< Recording time */

  /* TDRL */ ID3FID_RELEASETIME,       /**< Release time */

  /* TDTG */ ID3FID_TAGGINGTIME,       /**< Tagging time */

  /* TIPL */ ID3FID_INVOLVEDPEOPLE2,   /**< Involved people list */

  /* TENC */ ID3FID_ENCODEDBY,         /**< Encoded by */

  /* TEXT */ ID3FID_LYRICIST,          /**< Lyricist/Text writer */

  /* TFLT */ ID3FID_FILETYPE,          /**< File type */

  /* TIME */ ID3FID_TIME,              /**< Time */

  /* TIT1 */ ID3FID_CONTENTGROUP,      /**< Content group description */

  /* TIT2 */ ID3FID_TITLE,             /**< Title/songname/content description */

  /* TIT3 */ ID3FID_SUBTITLE,          /**< Subtitle/Description refinement */

  /* TKEY */ ID3FID_INITIALKEY,        /**< Initial key */

  /* TLAN */ ID3FID_LANGUAGE,          /**< Language(s) */

  /* TLEN */ ID3FID_SONGLEN,           /**< Length */

  /* TMCL */ ID3FID_MUSICIANCREDITLIST,/**< Musician credits list */

  /* TMED */ ID3FID_MEDIATYPE,         /**< Media type */

  /* TMOO */ ID3FID_MOOD,              /**< Mood */

  /* TOAL */ ID3FID_ORIGALBUM,         /**< Original album/movie/show title */

  /* TOFN */ ID3FID_ORIGFILENAME,      /**< Original filename */

  /* TOLY */ ID3FID_ORIGLYRICIST,      /**< Original lyricist(s)/text writer(s) */

  /* TOPE */ ID3FID_ORIGARTIST,        /**< Original artist(s)/performer(s) */

  /* TORY */ ID3FID_ORIGYEAR,          /**< Original release year */

  /* TOWN */ ID3FID_FILEOWNER,         /**< File owner/licensee */

  /* TPE1 */ ID3FID_LEADARTIST,        /**< Lead performer(s)/Soloist(s) */

  /* TPE2 */ ID3FID_BAND,              /**< Band/orchestra/accompaniment */

  /* TPE3 */ ID3FID_CONDUCTOR,         /**< Conductor/performer refinement */

  /* TPE4 */ ID3FID_MIXARTIST,         /**< Interpreted, remixed, or otherwise modified by */

  /* TPOS */ ID3FID_PARTINSET,         /**< Part of a set */

  /* TPRO */ ID3FID_PRODUCEDNOTICE,    /**< Produced notice */

  /* TPUB */ ID3FID_PUBLISHER,         /**< Publisher */

  /* TRCK */ ID3FID_TRACKNUM,          /**< Track number/Position in set */

  /* TRDA */ ID3FID_RECORDINGDATES,    /**< Recording dates */

  /* TRSN */ ID3FID_NETRADIOSTATION,   /**< Internet radio station name */

  /* TRSO */ ID3FID_NETRADIOOWNER,     /**< Internet radio station owner */

  /* TSIZ */ ID3FID_SIZE,              /**< Size */

  /* TSOA */ ID3FID_ALBUMSORTORDER,    /**< Album sort order */

  /* TSOP */ ID3FID_PERFORMERSORTORDER,/**< Performer sort order */

  /* TSOT */ ID3FID_TITLESORTORDER,    /**< Title sort order */

  /* TSRC */ ID3FID_ISRC,              /**< ISRC (international standard recording code) */

  /* TSSE */ ID3FID_ENCODERSETTINGS,   /**< Software/Hardware and settings used for encoding */

  /* TSST */ ID3FID_SETSUBTITLE,       /**< Set subtitle */

  /* TXXX */ ID3FID_USERTEXT,          /**< User defined text information */

  /* TYER */ ID3FID_YEAR,              /**< Year */

  /* UFID */ ID3FID_UNIQUEFILEID,      /**< Unique file identifier */

  /* USER */ ID3FID_TERMSOFUSE,        /**< Terms of use */

  /* USLT */ ID3FID_UNSYNCEDLYRICS,    /**< Unsynchronized lyric/text transcription */

  /* WCOM */ ID3FID_WWWCOMMERCIALINFO, /**< Commercial information */

  /* WCOP */ ID3FID_WWWCOPYRIGHT,      /**< Copyright/Legal infromation */

  /* WOAF */ ID3FID_WWWAUDIOFILE,      /**< Official audio file webpage */

  /* WOAR */ ID3FID_WWWARTIST,         /**< Official artist/performer webpage */

  /* WOAS */ ID3FID_WWWAUDIOSOURCE,    /**< Official audio source webpage */

  /* WORS */ ID3FID_WWWRADIOPAGE,      /**< Official internet radio station homepage */

  /* WPAY */ ID3FID_WWWPAYMENT,        /**< Payment */

  /* WPUB */ ID3FID_WWWPUBLISHER,      /**< Official publisher webpage */

  /* WXXX */ ID3FID_WWWUSER,           /**< User defined URL link */

  /*      */ ID3FID_METACRYPTO,        /**< Encrypted meta frame (id3v2.2.x) */

  /*      */ ID3FID_METACOMPRESSION,   /**< Compressed meta frame (id3v2.2.1) */

  /* >>>> */ ID3FID_LASTFRAMEID,       /**< Last field placeholder */

  ID3FID_MAX_COUNT,

};

////定义读取 MP3 ID3V2 所需要的结构体

typedef struct tag_MP3ID3V2INFO

{

  char   Identify[3];    //ID3三个字母。这里可以用来鉴别是不是文件信息内容

  struct

  {

    UCHAR majorversion;   //主版本号

    UCHAR revision;       //次版本号

  }       Version;

  UCHAR   HeaderFlags;

  UCHAR   HeaderSize[4];  //Note:每byte数值不能大于0x80,即该size值的每个bit 7均为0

  //上面的信息，共 10 个字节，成为 ID3V2 的头信息

  BOOL    bHasExtHeader;  //该 ID3V2 信息中是否含有扩展头部

  BOOL    bHasExtFooter;  //该 ID3V2 信息中是否含有扩展尾部

  DWORD   dwInfoSize;     //ID3V2 信息的大小

  std::string   ppszTagInfo[ID3FID_MAX_COUNT];  //求出来的 Tag 信息。如果存在则ppszTagInfo[]!=NULL

      //注意: 如果是要 Comment 信息，那么请从第4字节读起，也就是使用LPCSTR pszComment = (LPCSTR)&ppszTagInfo[ID3FID_COMMENT][4];

      //备注是以：eng\0开始的，所以应该向后搓4个字节

  BOOL    bHasTag;        //这个 MP3 中是否包含 ID3V2 信息

}MP3ID3V2INFO, *PMP3ID3V2INFO; 

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

int AudioInfo_GetMP3ID3V1(FILE *lpMP3File, MP3ID3V1INFO& MP3ID3V1Info);

 

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

int AudioInfo_GetMP3ID3V2(FILE *lpMP3File, MP3ID3V2INFO& MP3ID3V2Info);
