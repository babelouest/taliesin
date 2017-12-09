# Taliesin API Documentation

Here is the full documentaition of the REST API.

All data returned are in JSON format.

The prefix is `/api` by default, so this value will be used in this document, but you can change this in the `taliesin.config` file.

## Authentication

Most endpoints relies on a valid Glewlwyd Bearer token in the header, with a valid signature. For all requests, the username is provided by the Bearer token.

## Data source management

### List data source available for the user

#### URL

`/data_source/`

#### Method

`GET`

#### Success response

Code 200

Content
```javascript
[
  {
    "name": string, max 128 characters
    "description": string, max 512 characters
    "last_updated": integer, epoch time of the last refresh
    "scope": string, values available are "me" or "all"
  }
]
```

#### Error Response

Code 500

Internal Error

### Get a data source available for the user

#### URL

`/data_source/@data_source_name`

#### Method

`GET`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name

#### Success response

Code 200

Content
```javascript
{
  "name": string, max 128 characters
  "description": string, max 512 characters
  "last_updated": integer, epoch time of the last refresh
  "scope": string, values available are "me" or "all"
}
```

#### Error Response

Code 404

Data source not found

Code 500

Internal Error

### Add a new data source

#### URL

`/data_source/`

#### Method

`POST`

#### Data Parameters

```javascript
{
  "name": string, max 128 characters, mandatory, must be unique for the current user, including data source for all users
  "description": string, max 512 characters, optional
  "path": string, max 1024 characters, mandatory, must correspond to a valid path on the server
  "scope": string, optional, values available are "me" or "all", default "me", only an administrator can create a data source with the scope "all"
}
```

#### Success response

Code 200

#### Error Response

Code 500

Internal Error

OR

Code 400

Error input parameters

Content: json array containing all errors

### Update an existing data source

Note: By design, the user can't change the data source path, name or scope

#### URL

`/data_source/@data_source_name`

#### Method

`PUT`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name

#### Data Parameters

```javascript
{
  "description": string, max 512 characters, optional
}
```

#### Success response

Code 200

#### Error Response

Code 500

Internal Error

OR

Code 400

Error input parameters

Content: json array containing all errors

### Delete an existing data source

#### URL

`/data_source/@data_source_name`

#### Method

`DELETE`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name

#### Success response

Code 200

#### Error Response

Code 404

Data source not found

Code 403

Unauthorized to delete the specified data source

Code 500

Internal Error

### Refresh a data source

#### URL

`/data_source/@data_source_name/refresh/@relative_path`

#### Method

`PUT`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name

**Optional**

`@relative_path`: Relative path to refresh, default is `/`

#### Success response

Code 200

#### Error Response

Code 404

Data source not found

Code 403

Unauthorized to refresh the specified data source

Code 500

Internal Error

### Get the refresh status of a data source

#### URL

`/data_source/@data_source_name/refresh`

#### Method

`GET`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name

#### Success response

Code 200

Content
```javascript
{
  "status": string, values available are "not_running", "pending", "preparing", "running" and "error"
	"read": integer, number of files currently read
	"total": integer, total number of files to refresh
}
```

#### Error Response

Code 404

Data source not found

Code 403

Unauthorized to refresh the specified data source

Code 500

Internal Error

### Stop the current refresh of a data source

#### URL

`/data_source/@data_source_name/refresh/`

#### Method

`DELETE`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name

#### Success response

Code 200

#### Error Response

Code 404

Data source not found

Code 403

Unauthorized to refresh the specified data source

Code 500

Internal Error

### Clean a data source

#### URL

`/data_source/@data_source_name/clean`

#### Method

`POST`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name

#### Success response

Code 200

#### Error Response

Code 404

Data source not found

Code 403

Unauthorized to clean the specified data source

Code 500

Internal Error

## Media browse

### Browse a folder

#### URL

`/data_source/@data_source/@data_source_name/browse/path/@relative_path_to_folder`

Example: `/data_source/dataSource1/browse/path/Open Goldberg Variation/`

#### Method

`GET`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name

**Optional**

`@relative_path_to_folder`: Relative path to a folder, default is `/`

#### Success response

Code 200

Content
```javascript
[
  { // Example subfolder
    "name": string
    "path": string
    "type": "folder"
  },
  { // Example file
    "name": string
    "path": string
    "type": string, values can be "audio", "video", "image", "other"
    "last_updated": integer, epoch timestamp when the file was last updated in the filesystem
    "tags": {
      "key": "value" // Set of tags using the pair key/value
    }
  }
]
```

#### Error Response

Code 404

Path not found

Code 500

Internal Error

### Get a file

#### URL

`/data_source/@data_source/@data_source_name/browse/path/@relative_path_to_file`

#### Method

`GET`

**Required**

`@data_source_name`: Data Source name
`@relative_path_to_folder`: Relative path to a file

#### Success response

Code 200

Content
```javascript
{
  "name": string
  "path": string
  "type": string, values can be "audio", "video", "image", "other"
  "last_updated": integer, epoch timestamp when the file was last updated in the filesystem
  "tags": {
    "key": "value" // Set of tags using the pair key/value
  }
}
```

#### Error Response

Code 404

Path not found

Code 500

Internal Error

### Browse a category

#### URL

`/data_source/@data_source/@data_source_name/category/@level`

#### Method

`GET`

**Required**

`@data_source_name`: Data Source name
`@level`: level name of the category, values available are `artist`, `album`, `year`, `genre`

#### Success response

Code 200

Content
```javascript
[ // Array of strings of all the categories available for the selected level
  "cat1",
  "cat2",
  "cat3"
]
```

#### Error Response

Code 404

Level not found

Code 500

Internal Error

### Browse media files of a category

#### URL

`/data_source/@data_source/@data_source_name/category/@level/@category`

#### Method

`GET`

**Required**

`@data_source_name`: Data Source name
`@level`: level name of the category, values available are `artist`, `album`, `year`, `genre`
`@category`: category name

#### Success response

Code 200

Content
```javascript
[
  { // Example media
    "name": string
    "path": string
    "type": string, values can be "audio", "video"
    "last_updated": integer, epoch timestamp when the file was last updated in the filesystem
    "tags": {
      "key": "value" // Set of tags using the pair key/value
    }
  }
]
```

#### Error Response

Code 404

Level or category not found

Code 500

Internal Error

### Get category informations

#### URL

`/data_source/@data_source/@data_source_name/category/@level/@category/info`

#### Method

`GET`

**Required**

`@data_source_name`: Data Source name
`@level`: level name of the category, values available are `artist`, `album`, `year`, `genre`
`@category`: category name

#### Success response

Code 200

Content
```javascript
{
  "content": string, content of the category info
}
```

#### Error Response

Code 404

Level or category not found

Code 500

Internal Error

### Set category informations

#### URL

`/data_source/@data_source/@data_source_name/category/@level/@category/info`

#### Method

`GET`

**Required**

`@data_source_name`: Data Source name
`@level`: level name of the category, values available are `artist`, `album`, `year`, `genre`
`@category`: category name

#### Data Parameters

```javascript
{
  "content": string, max 16MB, optional
  "cover": string, max 16MB, optional
}
```

#### Success response

Code 200

#### Error Response

Code 404

Level or category not found

Code 500

Internal Error

Code 400

Error input parameters

Content: json array containing all errors

### Browse subcategories for a category

#### URL

`/data_source/@data_source/@data_source_name/category/@level/@category/@sublevel`

#### Method

`GET`

**Required**

`@data_source_name`: Data Source name
`@level`: level name of the category, values available are `artist`, `album`, `year`, `genre`
`@category`: category name
`@sublevel`: sublevel name, values available are `artist`, `album`, `year`, `genre`, must be different from `@level`

#### Success response

Code 200

Content
```javascript
[ // Array of strings of all the categories available for the selected level
  "cat1",
  "cat2",
  "cat3"
]
```

#### Error Response

Code 404

Level or category or sublevel not found

Code 500

Internal Error

### Browse media files of a subcategory

#### URL

`/data_source/@data_source/@data_source_name/category/@level/@category/@sublevel/@subcategory`

#### Method

`GET`

**Required**

`@data_source_name`: Data Source name
`@level`: level name of the category, values available are `artist`, `album`, `year`, `genre`
`@category`: category name
`@sublevel`: sublevel name, values available are `artist`, `album`, `year`, `genre`, must be different from `@level`
`@subcategory`: subcategory name

#### Success response

Code 200

Content
```javascript
[
  { // Example media
    "name": string
    "path": string
    "type": string, values can be "audio", "video"
    "last_updated": integer, epoch timestamp when the file was last updated in the filesystem
    "tags": {
      "key": "value" // Set of tags using the pair key/value
    }
  }
]
```

#### Error Response

Code 404

Level, category, subevel or subcategory not found

Code 400

Error in url parameters

Code 500

Internal Error

## Playlists management

### List playlists available for the user

#### URL

`/playlist/`

#### Method

`GET`

#### Success response

Code 200

Content
```javascript
[
  {
    "name": string, max 128 characters
    "description": string, max 512 characters
    "scope": string, values available are "me" or "all"
    "elements": integer, number of media files in the playlist
		"media": [ Array of media files in theplaylist
			{...}
		],
		"stream": [ // List of streams attached to this playlist
			{
				"name": string, name of the string
				"display_name": string, display name of the stream
				"webradio": boolean
			}
		]
  }
]
```

#### Error Response

Code 500

Internal Error

### Get a playlist available for the user

#### URL

`/playlist/@playlist_name`

#### Method

`GET`

#### URL Parameters

**Required**

`@playlist_name`: Playlist name

#### Success response

Code 200

Content
```javascript
{
	"name": string, max 128 characters
	"description": string, max 512 characters
	"scope": string, values available are "me" or "all"
	"elements": integer, number of media files in the playlist
	"media": [ Array of media files in theplaylist
		{...}
	],
	"stream": [ // List of streams attached to this playlist
		{
			"name": string, name of the string
			"display_name": string, display name of the stream
			"webradio": boolean
		}
	]
}
```

#### Error Response

Code 404

Playlist not found

Code 500

Internal Error

### Add a new playlist

#### URL

`/playlist/`

#### Method

`POST`

#### Data Parameters

```javascript
{
  "name": string, max 128 characters, mandatory, name of the  playlist, must be unique for the user
  "description": string, optional, max 512 characters
  "scope": string, values available are "me" or "all", optional, only an administrator can set a scope to "all", default is "me"
  "cover": string, cover image for the playlist in base64 encoding
  "media": [ // List of all media in the playlist
    {
      "data_source": data_source name of the media
      "path": relative path of the media
    }
  ]
}
```

#### Success response

Code 200

#### Error Response

Code 500

Internal Error

OR

Code 400

Error input parameters

Content: json array containing all errors

### Update an existing playlist

Note: By design, the user can't change the scope

#### URL

`/playlist/@playlist_name`

#### Method

`PUT`

#### URL Parameters

**Required**

`@playlist_name`: Playlist name

#### Data Parameters

```javascript
{
  "description": string, max 512 characters, optional
  "cover": string, cover image for the playlist in base64 encoding
  "media": [ // List of all media in the playlist
    {
      "data_source": data_source name of the media
      "path": relative path of the media
    }
  ]
}
```

#### Success response

Code 200

#### Error Response

Code 500

Internal Error

OR

Code 400

Error input parameters

Content: json array containing all errors

### Delete an existing playlist

#### URL

`/playlist/@playlist_name`

#### Method

`DELETE`

#### URL Parameters

**Required**

`@playlist_name`: Playlist name

#### Success response

Code 200

#### Error Response

Code 404

Playlist not found

Code 403

Unauthorized to delete the specified playlist

Code 500

Internal Error

### Add multiple media to an existing playlist

#### URL

`/playlist/@playlist_name/add_media`

#### Method

`PUT`

#### URL Parameters

**Required**

`@playlist_name`: Playlist name

#### Data Parameters

```javascript
[ // List of all media in the playlist
  {
    "data_source": data_source name of the media
    "path": relative path of the media
  }
]
```

#### Success response

Code 200

#### Error Response

Code 500

Internal Error

Code 404

Playlist not found

Code 403

Unauthorized to delete the specified playlist

OR

Code 400

Error input parameters

Content: json array containing all errors

### Remove multiple media to an existing playlist

#### URL

`/playlist/@playlist_name/delete_media`

#### Method

`DELETE`

#### URL Parameters

**Required**

`@playlist_name`: Playlist name

#### Data Parameters

```javascript
[ // List of all media in the playlist
  {
    "data_source": data_source name of the media
    "path": relative path of the media
  }
]
```

#### Success response

Code 200

#### Error Response

Code 500

Internal Error

Code 404

Playlist not found

Code 403

Unauthorized to delete the specified playlist

OR

Code 400

Error input parameters

Content: json array containing all errors

## Play media

All API that return a media or a list of media can be used to directly start a jukebox or a webradio by simply adding the query parameter `webradio` or `jukebox`. You can also add other optional parameters.

A webradio is a single stream that will play all songs at random or in sequence. A jukebox allows the user to choose what media should be played by using its index in the stream list.

### Play media based on a path, a category or a subcategory

#### URL

`/data_source/@data_source/@data_source_name/browse/path/@relative_path_to_folder?[webradio|jukebox][&random][&recursive][&format=][&bitrate=][&sample_rate=][&channels=][&name=]`

`/data_source/@data_source/@data_source_name/browse/category/@level/@category/@sublevel/@subcategory?[webradio|jukebox][&random][&format=][&bitrate=][&sample_rate=][&channels=][&name=]`

`/data_source/@data_source/@data_source_name/browse/category/@level/@category/@sublevel/@subcategory?[webradio|jukebox][&random][&format=][&bitrate=][&sample_rate=][&channels=][&name=]`

Examples: `/data_source/dataSource1/browse/path/Open Goldberg Variation?webradio&recursive`
          `/data_source/dataSource1/browse/path/Open Goldberg Variation/KIMIKO ISHIZAKA - Goldberg Variations BWV 988 - 01 - Aria__44k-24b.mp3?jukebox`
          `/data_source/dataSource1/browse/category/artist/Kimiko Ishizaka?jukebox&format=flac&sample_rate=48000`

#### Method

`GET`

#### URL Parameters

**Required**

`@data_source_name`: Data Source name
`@relative_path_to_folder`: Relative path to a folder, mandatory for `browse/path` urls only
`@level`: level name of the category, values available are `artist`, `album`, `year`, `genre`, mandatory for `browse/category` only
`@category`: category name, mandatory for `browse/category` only
`@sublevel`: sublevel name, values available are `artist`, `album`, `year`, `genre`, must be different from `@level`, optional for `browse/category` only
`@subcategory`: subcategory name, optional for `browse/category` only

***Query parameters***

**Mandatory**

Note: obviously you can't combine `webradio` and `jukebox` at the same time, if both parameters are set, the result stream will be a webradio.

`webradio`: Run a webradio stream based on the path or category parameters
`jukebox`: Run a jukebox that can be streamed based on the path or category parameters

**Optional**

***Parameters common to webradio and jukebox***

`format`: format used for the stream, values available are `mp3`, `vorbis` and `flac`
`bitrate`: bitrate used for the stream, for `mp3` or `vorbis` formats only, values available are 32000, 96000, 128000, 192000, 256000, 320000, if format `flac` is used, bitrate is automatically set to 1411000
`sample_rate`: sample rate (~frequency) used for the stream, values available are 8000, 11025, 22050, 32000, 44100, 48000
`channels`: number of channels used for the stream, values available are 1 (mono) or 2 (stereo)
`name`: display name for stream, optional, if not specified, the stream will have a display name corresponding to the path or the category

***Parameter used in webradio only***

`random`: if set, the webradio will play all media at random

***Parameter used in browse path only***

`recursive`: if set, all the subfolders will be included in the media list

#### Success response

Code 200

Content
```javascript
{
  "name": string, name of the stream, a 32 characters random sequence generated
  "display_name": string, display name of the stream
  "webradio": boolean true if the stream is a webradio
	"elements": number of elements in the stream
  "format": string, audio format of the stream, values available are `mp3`, `vorbis` or `flac`
  "stereo": boolean, true if the audio stream is stereo
  "sample_rate": integer, sample rate of the audio stream
  "bitrate": integer, bitrate of the audio stream
  "nb_client": integer, number of clients currently listening to the stream
  "last_seen": integer, date in epoch format when the stream was used last, for jukebox streams only
  "stored_playlist": string, name of the playlist the stream is based on, optional
	clients: [ Array of clients
		{
			"ip_address": string, ip address of the client
			"user_agent": string, user agent of the client
		}
	]
}
```

#### Error Response

Code 404

Path or category not found, or no media are available with the specified parameters

Code 400

Error input parameters 
 
Code 500

Internal Error

### Play a stored playlist

#### URL

`/playlist/@playlist_name/?[webradio|jukebox][&random][&format=][&bitrate=][&sample_rate=][&channels=][&name=]`

Examples: `/playlist/playlist1?webradio`
          `/playlist/playlist2?jukebox`
          `/playlist/playlist1?jukebox&format=flac&sample_rate=48000`

#### Method

`GET`

#### URL Parameters

**Required**

`@playlist_name`: Stored playlist name

***Query parameters***

**Mandatory**

Note: obviously you can't combine `webradio` and `jukebox` at the same time, if both parameters are set, the result will be a webradio.

`webradio`: Run a webradio stream based on the path or category parameters
`jukebox`: Run a jukebox that can be streamed based on the path or category parameters

**Optional**

***Parameters common to webradio and jukebox***

`format`: format used for the stream, values available are `mp3`, `vorbis` and `flac`
`bitrate`: bitrate used for the stream, for `mp3` or `vorbis` formats only, values available are 32000, 96000, 128000, 192000, 256000, 320000, if format `flac` is used, bitrate is automatically set to 1411000
`sample_rate`: sample rate (~frequency) used for the stream, values available are 8000, 11025, 22050, 32000, 44100, 48000
`channels`: number of channels used for the stream, values available are 1 (mono) or 2 (stereo)
`name`: display name for stream, optional, if not specified, the stream will have a display name corresponding to the path or the category

***Parameter used in webradio only***

`random`: if set, the webradio will play all media at random

***Parameter used in browse path only***

`recursive`: if set, all the subfolders will be included in the media list

#### Success response

Code 200

Content
```javascript
// Playlist response
{
  "name": string, name of the stream, a 32 characters random sequence generated
  "display_name": string, display name of the stream
  "webradio": boolean true if the stream is a webradio
	"elements": number of elements in the stream
  "format": string, audio format of the stream, values available are `mp3`, `vorbis` or `flac`
  "stereo": boolean, true if the audio stream is stereo
  "sample_rate": integer, sample rate of the audio stream
  "bitrate": integer, bitrate of the audio stream
  "nb_client": integer, number of clients currently listening to the stream
  "last_seen": integer, date in epoch format when the stream was used last, for jukebox streams only
  "stored_playlist": string, name of the playlist the stream is based on, optional
	clients: [ Array of clients
		{
			"ip_address": string, ip address of the client
			"user_agent": string, user agent of the client
		}
	]
}
```

#### Error Response

Code 404

Path or category not found, or no media are available with the specified parameters

Code 400

Error input parameters 
 
Code 500

Internal Error

## Get covers

You can get cover from media files, folders, playlists and categories The query parameter to add is `cover`

### Get a cover from a media file, a folder, a playlist or a category

#### URLs

`/data_source/@data_source/@data_source_name/browse/path/@relative_path?cover[&thumbnail][&base64]`
`/data_source/@data_source/@data_source_name/category/@level/@category?cover[&thumbnail][&base64]`
`/playlist/@playlist_name?cover[&thumbnail][&base64]`

#### Method

`GET`

#### URL Parameters

**Required**

`@data_source`: data source name, required for data source browse
`@relative_path`: relative path to folder or media file, required for data source path browse
`@level`: level name of the category, values available are `artist`, `album`, `year`, `genre`, required for data source category browse
`@category`: category name, required for data source category browse
`@playlist_name`: Playlist name, required for playlist browse

#### Success response

Code 200

Content: Raw cover in binary or base64

#### Error Response

Code 500

Internal Error

Code 404

Cover not found

## Search

### Simple search

Simple search allows to search by folder name, media file name, categories (artist, album, year, genre and title), stored playlist name, and stream display name. All the search criteria must be set in the url. The simple search will be executed in all data source available for the user. All returned list are limited to 100 elements per category.

#### URL

`/search?q=[search_pattern]&category=[playlist|stream|folder|file|title|title|artist|album|year|genre]`

#### Method

`GET`

#### URL Parameters

**Required**

`q`: string, search pattern to look for. Search is case insensitive and search if the pattern is contained in the data.

**Optional**

`category`: Search category, optional, values available are `playlist`, `stream`, `folder`, `file`, `title`, `title`, `artist`, `album`, `year`, `genre`, default is non, which means all categories.

#### Success response

Code 200

Content
```javascript
{
  "playlist": [ // Array of playlist names corresponding the pattern, optional
    "playlist1",
    "playlist2"
    //etc
  ],
  "stream": [ // Array of stream objects corresponding the pattern, optional
    {
      "name": string, name of the stream
      "display_name": string, display name of the stream
      "webradio": boolean, true if the stream is a webradio
    }
  ],
  "folder": [ // Array of folders whose name correspond the pattern, optional
    {
      "data_source": string, name of the data source
      "name": string, name of the folder
      "path": string, relative path of the folder
    }
  ],
  "media": [ // Array of media whose name or metadata correspond the pattern, optional
    {
      "data_source": string, name of the data source
      "name": string, name of the media file
      "path": string, relative path of the media file
    }
  ]
}
```

#### Error Response

Code 400

No pattern specified

Code 500

Internal Error

### Advanced search

Advanced search allows to find for media files only, you can specify multiple search criteria, all criteria will be evaluated with a `AND` combimation.

#### URL

`/search`

#### Method

`PUT`

#### Data Parameters

```javascript
{
  "limit": positive integer, optional, maximum number of elements returned, default is 100
  "offset": positive integer, optional, offset to start the list of elements returned
  "data_source": [ // Array of data source names to make the search, optional, if no data source is specified, the search will be executed in all data source available for the user
    "datasource1",
    "datasource2"
    // etc.
  ],
  "tags": [ // Array of tag search
    {
      "key": string, mandatory, key name to search
      "operator": string, mandatory, operator to use for the search pattern, values available are `equals`, `different`, `contains`, `lower`, `higher`, `between` and `empty`
      "value": string, mandatory if `operator` is not `empty`, otherwise ignored
      "value_max": string, mandatory if `operator` is `between`, otherwise ignored
    }
  ],
  "metrics": { // Object for media metrics
    "nb_play": { // Number of times the medi has been played
      "operator": string, mandatory, values available are `equals`, `different`, `lower`, `higher` and `between`
      "value": positive integer, mandatory
    }
    "played_at":  { // epoch time when the media was played
      "operator": string, mandatory, values available are `equals`, `different`, `lower`, `higher` and `between`
      "value": positive integer, mandatory
    }
    "last_seen":  { // epoch time when the media was last played
      "operator": string, mandatory, values available are `equals`, `different`, `lower`, `higher` and `between`
      "value": positive integer, mandatory
    }
  }
}
```

#### Success response

Code 200

Content
```javascript
[
  {
    "data_source": string, name of the data source
    "name": string, name of the media file
    "path": string, relative path of the media file
  }
]
```

#### Error Response

Code 400

No pattern specified

Code 500

Internal Error

## Streams

### List streams available for the user

#### URL

`/stream/`

#### Method

`GET`

#### Success response

Code 200

Content
```javascript
[
  {
		"name": string, name of the stream, a 32 characters random sequence generated
		"display_name": string, display name of the stream
		"webradio": boolean true if the stream is a webradio
		"elements": number of elements in the stream
		"format": string, audio format of the stream, values available are `mp3`, `vorbis` or `flac`
		"stereo": boolean, true if the audio stream is stereo
		"sample_rate": integer, sample rate of the audio stream
		"bitrate": integer, bitrate of the audio stream
		"nb_client": integer, number of clients currently listening to the stream
		"last_seen": integer, date in epoch format when the stream was used last, for jukebox streams only
		"stored_playlist": string, name of the playlist the stream is based on, optional
		clients: [ Array of clients
			{
				"ip_address": string, ip address of the client
				"user_agent": string, user agent of the client
			}
		]
  }
]
```

#### Error Response

Code 500

Internal Error

### Listen to a stream

For a stream to be available to usual media players, we can't use a Oauth2 Bearer token to authenticate the stream, because it's not supported, because it's way too overkill. Instead, all Taliesin stream address use a 32 bytes identifier which is randomly generated, so an unauthorized client needs to guess the stream name to access it.

#### URL

`/stream/@stream_name[?index=]`

#### Method

`GET`

#### URL Parameters

**Required**

`@stream_name`: string, stream name specified by the stream info

**Optional**

`index`: media index in the stream playlist, for jukebox streams only. If the stream is a jukebox and no index is speciied, the server will return a m3u file with all jukebox stream media.

#### Success response

Code 200

Binary data of the stream to be played by the client.
The stream is also compatible with ICY metatags, so the client application, e.g. VLC or MPD will display the current media information using the following pattern:

`[Artist - ][Album ][(Year) - ][Title|filename]`

Example: `Kimiko Ishizaka - Goldberg Variations BWV 988 (2012) - Variatio 2 a 1 Clav.`

#### Error Response

Code 404

Stream or index not found

Code 500

Internal Error

### Get a stream cover

For a stream cover to be available to usual media players, we can't use a Oauth2 Bearer token to authenticate the stream, because it's mostly not supported, and because it's way too overkill. Instead, all Taliesin stream address use a randomly generated alphanumeric identifier of 32 characters long, so an unauthorized client needs to guess the stream name to access it.

#### URL

`/stream/@stream_name/cover[?index=][&base64]`

#### Method

`GET`

#### URL Parameters

**Required**

`@stream_name`: string, stream name specified by the stream info

**Optional**

`index`: media index in the stream playlist, for playlist streams only.
`base64`: return cover in base64 encoding

#### Success response

Code 200

Binary data or base64 encoding of the stream cover.

#### Error Response

Code 404

Stream or index not found

Code 500

Internal Error

### Manage a stream with a REST API

#### URL

`/stream/@stream_name/manage`

#### Method

`PUT`

#### URL Parameters

**Required**

`@stream_name`: string, stream name specified by the stream info

#### Data Parameters

```javascript
{
  "command": string, name of the command, mandatory
  "parameters": JSON object or array to specify the parameters when needed
}
```

The commands available are:

- `stop`: stop the current stream and remove it from the stream list
- `replay`: replay the current media from the beginning, for webradio only
- `skip`: skip current media and immediatly play next media, for webradio only
- `history`: Return the list of media previously played in this stream, for webradio only
- `info`: Return the stream information
- `now`: Return the current media information, for webradio only
- `next`: Return the next media information, for webradio only
- `list`: Return the list of all media available in this stream
- `append_list`: Append a list of media in the current stream media list, parameter format is [{data_source: string, path: string, recursive: boolean},{category: string, category_value: string, sub_category: string, sub_category_value: string}]
- `remove_list`: Remove a media in the current stream media list, parameter format is {index: integer}
- `move`: Move a media in the stream list to a different index, parameter format is {index: integer, target: integer}
- `attach_playlist`: Attach the stream to a stored playlist, parameter format is {name: string}
- `reload`: Reload the stream media list with the attached stored playlist if one is specified

#### Success response

Code 200

Returned data if available, otherwise empty.

The returned data available are:

- Command `history`, result is in reverse chronological order
```javascript
[
  {
    "datestamp": string, epoch value when the media was played
    "media": {
      "data_source": string,
      "name": string
      "path": string
      "type": string
      "tags": {
        "key": "value"
      }
    }
  }
]
```

- Command `info`
```javascript
{
  "name": string, name of the stream, a 32 characters random sequence generated
  "display_name": string, display name of the stream
  "webradio": boolean true if the stream is a webradio
	"elements": number of elements in the stream
  "format": string, audio format of the stream, values available are `mp3`, `vorbis` or `flac`
  "stereo": boolean, true if the audio stream is stereo
  "sample_rate": integer, sample rate of the audio stream
  "bitrate": integer, bitrate of the audio stream
  "nb_client": integer, number of clients currently listening to the stream
  "last_seen": integer, date in epoch format when the stream was used last, for jukebox streams only
  "stored_playlist": string, name of the playlist the stream is based on, optional
	clients: [ Array of clients
		{
			"ip_address": string, ip address of the client
			"user_agent": string, user agent of the client
		}
	]
}
```javascript

- Command `now`
```javascript
{
  "data_source": string,
  "name": string
  "path": string
  "type": string
  "tags": {
    "key": "value"
  }
}
```

- Command `next`
```javascript
{
  "data_source": string,
  "name": string
  "path": string
  "type": string
  "tags": {
    "key": "value"
  }
}
```

- Command `list`
```javascript
[
  {
    "data_source": string,
    "name": string
    "path": string
    "type": string
    "tags": {
      "key": "value"
    }
  }
]
```

#### Error Response

Code 404

Stream not found

Code 403

User can't manage this stream

Code 500

Internal Error

### Manage a stream with a Websocket

If you feel like it, you can also manage a stream via a Websocket. The commands are exactly the same than with the REST API management, but must be sent to the server as a serialized JSON. The messages sent by the server to the client have the following format:

```javascript
{
  "command": string, command name
  "result": result of the command
}
```

Since it seems that most Web browser can't use Oauth2 Bearer tokens in headers to authentify a websocket, the client needs to send the bearer token right after the connection is established to allow communication. The message must have the following format:

```javascript
{
  "command": "authorization",
  "token": string, token value
}
```

Then, the token will be verified and if the token is valid, the client and the server will be able to send each other messages via the websocket.

The server will automatically send a result for the `now` command every time a new media is played. Also, the server will send a result for the `stop` command if the stream is stopped.

The other commands are the same as the REST API commands.

#### URL

`/stream/@stream_name/ws`

#### Method

`GET`

#### URL Parameters

**Required**

`@stream_name`: string, stream name specified by the stream info

#### Command messages parameters

```javascript
{
  "command": string, name of the command, mandatory
  "parameters": JSON object or array to specify the parameters when needed
}
```

The commands available are:

- `stop`: stop the current stream and remove it from the stream list
- `replay`: replay the current media from the beginning, for webradio only
- `skip`: skip current media and immediatly play next media, for webradio only
- `history`: Return the list of media previously played in this stream, for webradio only
- `info`: Return the stream information
- `now`: Return the current media information, for webradio only
- `next`: Return the next media information, for webradio only
- `list`: Return the list of all media available in this stream
- `append_list`: Append a list of media in the current stream media list, parameter format is [{data_source: string, path: string}]
- `remove_list`: Remove a media in the current stream media list, parameter format is {index: integer}
- `move`: Move a media in the stream list to a different index, parameter format is {index: integer, target: integer}
- `attach_playlist`: Attach the stream to a stored playlist, parameter format is {name: string}
- `reload`: Reload the stream media list with the attached stored playlist if one is specified

#### Success response

Code 200

Returned data if available, otherwise empty.

The returned data available are:

- Command `history`, result is in reverse chronological order
```javascript
{
  "command": "history"
  "result": [
    {
      "datestamp": string, epoch value when the media was played
      "media": {
        "data_source": string,
        "name": string
        "path": string
        "type": string
        "tags": {
          "key": "value"
        }
      }
    }
  ]
}

```

- Command `info`
```javascript
{
  "command": "info"
  "result":
	{
		"name": string, name of the stream, a 32 characters random sequence generated
		"display_name": string, display name of the stream
		"webradio": boolean true if the stream is a webradio
		"elements": number of elements in the stream
		"format": string, audio format of the stream, values available are `mp3`, `vorbis` or `flac`
		"stereo": boolean, true if the audio stream is stereo
		"sample_rate": integer, sample rate of the audio stream
		"bitrate": integer, bitrate of the audio stream
		"nb_client": integer, number of clients currently listening to the stream
		"last_seen": integer, date in epoch format when the stream was used last, for jukebox streams only
		"stored_playlist": string, name of the playlist the stream is based on, optional
		clients: [ Array of clients
			{
				"ip_address": string, ip address of the client
				"user_agent": string, user agent of the client
			}
		]
	}
}
```javascript

- Command `now`
```javascript
{
  "command": "now"
  "result": {
    "data_source": string,
    "name": string
    "path": string
    "type": string
    "tags": {
      "key": "value"
    }
  }
}
```

- Command `next`
```javascript
{
  "command": "next"
  "result": {
    "data_source": string,
    "name": string
    "path": string
    "type": string
    "tags": {
      "key": "value"
    }
  }
}
```

- Command `list`
```javascript
{
  "command": "next"
  "result": [
    {
      "data_source": string,
      "name": string
      "path": string
      "type": string
      "tags": {
        "key": "value"
      }
    }
  ]
}
```

#### Error Response

Code 404

Stream not found

Code 403

User can't manage this stream

Code 500

Internal Error
