import React, { Component } from 'react';
import { DropdownButton, Button, ButtonGroup, MenuItem, Table, Image, Row, Col } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ModalConfirm from '../Modal/ModalConfirm';
import ModalEditPlaylist from '../Modal/ModalEditPlaylist';
import ModalEditStream from '../Modal/ModalEditStream';

class BrowsePlaylist extends Component {	
  constructor(props) {
    super(props);
		
		this.state = {
			playlist: StateStore.getState().playlists,
			curPlaylist: {},
			isAdmin: StateStore.getState().profile.isAdmin,
			shownPlaylist: {},
      addPlaylistShow: false,
			modalEditShow: false,
			modalDeleteShow: false,
			editStreamShow: false,
      add: false,
			modalDeleteMessage: "",
      showPlaylist: false,
			playlistToShow: props.playlist,
      mediaList: [],
      offset: 0,
      limit: 100
		};
    
    this.getCovers();
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setPlaylists" || reduxState.lastAction === "setPlaylist") {
        this.setState({playlist: reduxState.playlists});
			}
		});

		this.getCovers = this.getCovers.bind(this);
		this.getPlaylistCover = this.getPlaylistCover.bind(this);
		this.canUpdate = this.canUpdate.bind(this);
		this.playNow = this.playNow.bind(this);
		this.playAdvanced = this.playAdvanced.bind(this);
		this.runPlaylistAdvanced = this.runPlaylistAdvanced.bind(this);
		this.showPlaylist = this.showPlaylist.bind(this);
		this.showList = this.showList.bind(this);
		this.addPlaylist = this.addPlaylist.bind(this);
		this.onAddPlaylist = this.onAddPlaylist.bind(this);
		this.editPlaylist = this.editPlaylist.bind(this);
		this.deletePlaylist = this.deletePlaylist.bind(this);
		this.onEditPlaylist = this.onEditPlaylist.bind(this);
		this.onDeletePlaylist = this.onDeletePlaylist.bind(this);
		this.onCloseStreamModal = this.onCloseStreamModal.bind(this);
		this.onSavePlaylist = this.onSavePlaylist.bind(this);
		this.getMediaCovers = this.getMediaCovers.bind(this);
		this.getMediaCover = this.getMediaCover.bind(this);
		this.deleteMedia = this.deleteMedia.bind(this);
		this.refreshPlaylists = this.refreshPlaylists.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			playlist: StateStore.getState().playlists,
			curPlaylist: {},
			isAdmin: StateStore.getState().profile.isAdmin,
			shownPlaylist: {},
      addPlaylistShow: false,
			modalEditShow: false,
			modalDeleteShow: false,
			editStreamShow: false,
      add: false,
			modalDeleteMessage: "",
      showPlaylist: false,
			playlistToShow: nextProps.playlist,
      mediaList: [],
      offset: 0,
      limit: 100
		}, () => {
      this.getCovers();
			if (this.state.playlistToShow) {
				var playlist = this.state.playlistToShow;
				this.setState({playlistToShow: false}, () => {
					this.showPlaylist(playlist);
				});
			}
    });
	}
  
	componentDidMount() {
		this._ismounted = true;
		if (this.state.playlistToShow) {
			var playlist = this.state.playlistToShow;
			this.setState({playlistToShow: false}, () => {
				this.showPlaylist(playlist);
			});
		}
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
  getCovers() {
		if (this._ismounted) {
			var list = this.state.playlist;
			for (var i in list) {
				this.getPlaylistCover(list[i].name);
			}
			this.setState({playlist: list});
		}
  }
  
  getPlaylistCover(name) {
		if (this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + name + "?cover&thumbnail&base64")
			.then((cover) => {
				var list = this.state.playlist;
				for (var i in list) {
					if (list[i].name === name) {
						list[i].cover = cover;
						this.setState({playlist: list});
						break;
					}
				}
			});
		}
  }
  
	showPlaylist(playlist) {
		if (this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + playlist.name + "?offset=" + this.state.offset + "&limit=" + this.state.limit)
			.then((pl) => {
				this.setState({showPlaylist: true, shownPlaylist: playlist, mediaList: pl.media}, () => {
					this.getMediaCovers();
				});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error getting playlist',
					level: 'error'
				});
			});
		}
	}
	
	showList() {
		if (this._ismounted) {
			this.setState({showPlaylist: false, shownPlaylist: {}, mediaList: []});
		}
	}
  
  getMediaCovers() {
    var list = this.state.mediaList;
    for (var i in list) {
      this.getMediaCover(list[i]);
    }
  }
  
  getMediaCover(media) {
		if (this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + media.data_source + "/browse/path/" + media.path + "?cover&thumbnail&base64")
			.then((cover) => {
				var list = this.state.mediaList;
				for (var i in list) {
					if (list[i].data_source === media.data_source && list[i].path === media.path) {
						list[i].cover = cover;
						this.setState({mediaList: list});
					}
				}
			});
		}
  }
	
  addPlaylist() {
		if (this._ismounted) {
			this.setState({addPlaylistShow: true, add: true});
		}
  }
  
  onAddPlaylist(playlist) {
    if (playlist && this._ismounted) {
      StateStore.getState().APIManager.taliesinApiRequest("POST", "/playlist/", playlist)
      .then(() => {
        var list = this.state.playlist
        list.push(playlist);
        StateStore.dispatch({type: "setPlaylists", playlists: list});
        this.setState({playlist: list, curPlaylist: false});
        StateStore.getState().NotificationManager.addNotification({
          message: 'Playlist added',
          level: 'info'
        });
      })
      .fail(() => {
        StateStore.getState().NotificationManager.addNotification({
          message: 'Error adding playlist',
          level: 'error'
        });
      });
    }
  }
  
  onSavePlaylist(result, playlist, add) {
		if (this._ismounted) {
			if (result) {
				if (add) {
					this.setState({addPlaylistShow: false}, () => {
						this.onAddPlaylist(playlist);
					});
				} else {
					this.setState({addPlaylistShow: false}, () => {
						this.onEditPlaylist(playlist);
					});
				}
			} else {
				this.setState({addPlaylistShow: false});
			}
		}
  }
	
	canUpdate(playlist) {
		return this.state.isAdmin || (playlist.scope === "me");
	}
	
  playNow(playlist) {
		var streamList = StateStore.getState().streamList, curStream = streamList.find((stream) => {return stream.display_name.startsWith("{" + (StateStore.getState().profile.currentPlayer||"local") + "}")});
		if (curStream) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(curStream.name) + "/manage", {command: "stop"})
			.then(() => {
				var streamList = StateStore.getState().streamList;
				for (var i in streamList) {
					if (streamList[i].name === curStream.name) {
						streamList.splice(i, 1);
						break;
					}
				}
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				if (StateStore.getState().profile.stream.name === curStream.name) {
					StateStore.dispatch({type: "loadStream", stream: false});
				}
				this.runPlayElement(playlist);
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error delete stream',
					level: 'error'
				});
			});
		} else {
			this.runPlayElement(playlist);
		}
  }
	
	runPlayElement(playlist) {
    StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(playlist.name) + "/load?jukebox&name={" + (StateStore.getState().profile.currentPlayer||"local") + "} - " + playlist.name)
    .then((result) => {
			var streamList = StateStore.getState().streamList;
      streamList.push(result);
      StateStore.dispatch({type: "setStreamList", streamList: streamList});
      StateStore.dispatch({type: "loadStreamAndPlay", stream: result, index: 0});
			StateStore.getState().NotificationManager.addNotification({
				message: 'Play new stream',
				level: 'info'
			});
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error Play',
				level: 'error'
			});
    });
	}
  
	playAdvanced(playlist) {
		if (this._ismounted) {
			this.setState({
				curPlaylist: playlist,
				editStreamShow: true
      });
		}
	}
  
  runPlaylistAdvanced(player) {
		if (player) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(this.state.curPlaylist.name) + "/load?" + player.type + (player.recursive?"&recursive":"") + "&format=" + player.format + "&channels=" + player.channels + "&bitrate=" + player.bitrate + "&sample_rate=" + player.sampleRate + (player.random?"&random":"") + "&name=" + this.state.curPlaylist.name)
			.then((result) => {
				var streamList = StateStore.getState().streamList;
				streamList.push(result);
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				StateStore.dispatch({type: (player.playNow?"loadStreamAndPlay":"loadStream"), stream: result, index: 0});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error Play stream',
					level: 'error'
				});
			});
		}
    this.setState({editStreamShow: false});
  }
	
	editPlaylist(playlist) {
		if (this._ismounted) {
			this.setState({addPlaylistShow: true, add: false, curPlaylist: playlist});
		}
	}
	
	onEditPlaylist(playlist) {
    if (playlist && this._ismounted) {
      StateStore.getState().APIManager.taliesinApiRequest("PUT", "/playlist/" + playlist.name, playlist)
      .then(() => {
        var list = this.state.playlist
        for (var i in list) {
          if (list[i].name === playlist.name) {
            list[i] = playlist;
          }
        }
        StateStore.dispatch({type: "setPlaylists", playlists: list});
        this.setState({playlist: list, curPlaylist: false});
        StateStore.getState().NotificationManager.addNotification({
          message: 'Playlist updated',
          level: 'info'
        });
      })
      .fail(() => {
        StateStore.getState().NotificationManager.addNotification({
          message: 'Error updating playlist',
          level: 'error'
        });
      });
    }
	}
	
	deletePlaylist(playlist) {
		if (this._ismounted) {
			this.setState({modalDeleteShow: true, modalDeleteMessage: "Are you sure you want to delete the playlist '" + playlist.name + "'?", curPlaylist: playlist});
		}
	}
	
	onDeletePlaylist(result) {
    if (result && this._ismounted) {
      StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/playlist/" + encodeURIComponent(this.state.curPlaylist.name))
      .then((result) => {
        var list = this.state.playlist
        list.splice(this.state.playlist.indexOf(this.state.curPlaylist), 1);
        StateStore.dispatch({type: "setPlaylists", playlists: list});
        this.setState({modalDeleteShow: false, playlist: list, curPlaylist: false});
        StateStore.getState().NotificationManager.addNotification({
          message: 'Playlist deleted',
          level: 'info'
        });
      })
      .fail(() => {
        StateStore.getState().NotificationManager.addNotification({
          message: 'Error delete playlist',
          level: 'error'
        });
      });
    }
	}
	
	onCloseStreamModal(player) {
		if (this._ismounted) {
			if (player) {
				StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(this.state.curPlaylist.name) + "/load?" + player.type + "&format=" + player.format + "&channels=" + player.channels + "&bitrate=" + player.bitrate + "&sample_rate=" + player.sampleRate)
				.then((result) => {
					var streamList = StateStore.getState().streamList;
					streamList.push(result);
					StateStore.dispatch({type: "setStreamList", streamList: streamList});
					StateStore.dispatch({type: "loadStream", stream: result});
					StateStore.getState().NotificationManager.addNotification({
						message: 'Play new stream',
						level: 'info'
					});
				})
				.fail(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: 'Error Play stream',
						level: 'error'
					});
				});
			}
			this.setState({editStreamShow: false});
		}
	}
	
	deleteMedia(media) {
		if (this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/playlist/" + this.state.shownPlaylist.name, {data_source: media.data_source, path: media.path})
			.then(() => {
				var list = this.state.mediaList;
				list.splice(list.indexOf(media), 1);
				this.setState({mediaList: list});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error deleting media',
					level: 'error'
				});
			});
		}
	}
	
	refreshPlaylists() {
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist")
		.then((result) => {
			StateStore.dispatch({type: "setPlaylists", playlists: result});
      StateStore.getState().NotificationManager.addNotification({
        message: 'Plylists refreshed',
        level: 'info'
      });
			this.getCovers();
		})
		.fail((result) => {
			StateStore.dispatch({type: "setPlaylists", playlists: []});
      StateStore.getState().NotificationManager.addNotification({
        message: 'Error refreshing playlists',
        level: 'error'
      });
		});
	}
	
  render() {
    if (!this.state.showPlaylist) {
      var displayList = [];
      this.state.playlist.forEach((aPlaylist, index) => {
        var cover;
        if (aPlaylist.cover) {
          cover = <Image src={"data:image/jpeg;base64," + aPlaylist.cover} responsive style={{maxWidth: "100px", maxHeight: "100px"}}/>
        }
        displayList.push(
          <tr key={index}>
            <td className="text-center">
              {cover}
            </td>
            <td>
              <a role="button" onClick={() => this.showPlaylist(aPlaylist)}>{aPlaylist.name}</a>
            </td>
            <td className="hidden-xs">
              {aPlaylist.description}
            </td>
            <td className="hidden-xs">
              {aPlaylist.elements||"N/A"}
            </td>
            <td>
              {aPlaylist.scope==="all"?<FontAwesome name={"users"} />:<FontAwesome name={"user"} />}
            </td>
            <td className="text-center">
              <ButtonGroup className="hidden-xs">
                <Button title="Play now" onClick={() => {this.playNow(aPlaylist)}}>
                  <FontAwesome name={"play"} />
                </Button>
                <Button title="Create stream" onClick={() => {this.playAdvanced(aPlaylist)}}>
                  <FontAwesome name={"play"} />&nbsp;
                  <FontAwesome name={"cog"} />
                </Button>
                <Button title="Edit" onClick={() => this.editPlaylist(aPlaylist)} disabled={!this.canUpdate(aPlaylist)}>
                  <FontAwesome name={"pencil"} />
                </Button>
                <Button title="Delete" onClick={() => this.deletePlaylist(aPlaylist)} disabled={!this.canUpdate(aPlaylist)}>
                  <FontAwesome name={"trash"} />
                </Button>
              </ButtonGroup>
              <DropdownButton className="visible-xs" id={"xs-manage"-aPlaylist.name} pullRight title={
                <span><i className="fa fa-cog"></i></span>
              }>
                <MenuItem onClick={() => {this.playNow(aPlaylist)}}>
                  <FontAwesome name={"play"} />&nbsp;
                  Play now
                </MenuItem>
                <MenuItem divider />
                <MenuItem onClick={() => {this.playAdvanced(aPlaylist)}}>
                  <FontAwesome name={"play"} />
                  <FontAwesome name={"cog"} />&nbsp;
                  Create stream
                </MenuItem>
                <MenuItem onClick={() => this.editPlaylist(aPlaylist)} disabled={!this.canUpdate(aPlaylist)}>
                  <FontAwesome name={"pencil"} />&nbsp;
                  Edit
                </MenuItem>
                <MenuItem onClick={() => this.deletePlaylist(aPlaylist)} disabled={!this.canUpdate(aPlaylist)}>
                  <FontAwesome name={"trash"} />&nbsp;
                  Delete
                </MenuItem>
              </DropdownButton>
            </td>
          </tr>
        );
      });
      return (
        <div>
					<ButtonGroup>
						<Button title="Add a new playlist" onClick={() => this.addPlaylist()}>
							<FontAwesome name={"plus"} />
						</Button>
						<Button title="Refresh list" onClick={() => this.refreshPlaylists()}>
							<FontAwesome name={"refresh"} />
						</Button>
					</ButtonGroup>
          <Table striped bordered condensed hover>
            <thead>
              <tr>
                <th>
                  Icon
                </th>
                <th>
                  Name
                </th>
                <th className="hidden-xs">
                  Description
                </th>
                <th className="hidden-xs">
                  Elements
                </th>
                <th>
                  Scope
                </th>
                <th>
                </th>
              </tr>
            </thead>
            <tbody>
              {displayList}
            </tbody>
          </Table>
          <ModalConfirm show={this.state.modalDeleteShow} title={"Delete playlist"} message={this.state.modalDeleteMessage} onCloseCb={this.onDeletePlaylist}/>
          <ModalEditPlaylist show={this.state.addPlaylistShow} onCloseCb={this.onSavePlaylist} add={this.state.add} playlist={this.state.curPlaylist} />
          <ModalEditStream 
            show={this.state.editStreamShow} 
            playlist={this.state.curPlaylist} 
            onCloseCb={this.runPlaylistAdvanced} 
          />
        </div>
      );
    } else {
      var mediaList = [];
      this.state.mediaList.forEach((media, index) => {
        var cover;
        if (media.cover) {
          cover = <Image src={"data:image/jpeg;base64," + media.cover} responsive style={{maxWidth: "100px", maxHeight: "100px"}}/>
        }
        mediaList.push(
          <tr key={index}>
            <td>
              {cover}
            </td>
            <td>
              {media.data_source}
            </td>
            <td>
              {media.path}
            </td>
            <td>
              <ButtonGroup>
                <Button title="Delete element" onClick={() => this.deleteMedia(media)}>
                  <FontAwesome name={"trash"} />
                </Button>
              </ButtonGroup>
            </td>
          </tr>
        );
      });
      return (
				<div>
					<Row>
						<Col md={12} sm={12} xs={12}>
							<Button title="Show all playlists" onClick={() => this.showList()}>
								<FontAwesome name={"list"} />
							</Button>
						</Col>
					</Row>
					<Table striped bordered condensed hover>
						<thead>
							<tr>
								<th>
									Cover
								</th>
								<th>
									Data Source
								</th>
								<th>
									Path
								</th>
								<th>
								</th>
							</tr>
						</thead>
						<tbody>
							{mediaList}
						</tbody>
					</Table>
				</div>
      );
    }
	}
}

export default BrowsePlaylist;
