import React, { Component } from 'react';
import { Modal, Row, Col, Image, Button, ButtonGroup, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class ModalMedia extends Component {
  constructor(props) {
    super(props);
		
		this.state = {
      show: props.show, 
      media: props.media, 
      title: props.title, 
      close: props.onClose, 
      imgBlob: false,
			streamList: StateStore.getState().streamList,
			playlist: StateStore.getState().playlists
    };
		
		this.onCloseModal = this.onCloseModal.bind(this);
		this.getMediaCover = this.getMediaCover.bind(this);
		this.handleSelectDataSource = this.handleSelectDataSource.bind(this);
		this.handleSelectArtist = this.handleSelectArtist.bind(this);
		this.handleSelectAlbum = this.handleSelectAlbum.bind(this);
		this.handleSelectYear = this.handleSelectYear.bind(this);
		this.handleSelectGenre = this.handleSelectGenre.bind(this);
		this.onPlayNow = this.onPlayNow.bind(this);
		this.runPlayNow = this.runPlayNow.bind(this);
		this.addToNewPlaylist = this.addToNewPlaylist.bind(this);
		this.onSavePlaylist = this.onSavePlaylist.bind(this);
		
		this.getMediaCover();
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
      show: nextProps.show, 
      media: nextProps.media, 
      title: nextProps.title, 
      close: nextProps.onClose, 
      imgBlob: false,
			streamList: StateStore.getState().streamList,
			playlist: StateStore.getState().playlists
    }, () => {
			this.getMediaCover();
		});
	}

  onCloseModal() {
		this.setState({show: false}, () => {
			if (this.state.close) {
				this.state.close();
			}
		});
  }
	
	onPlayNow() {
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
				this.runPlayNow();
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("common.message_error_delete_stream"),
					level: 'error'
				});
			});
		} else {
			this.runPlayNow();
		}
	}
	
	runPlayNow() {
    StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23") + "?jukebox&recursive&name={" + (StateStore.getState().profile.currentPlayer||"local") + "} - " + (this.state.media.tags.title||this.state.media.name))
    .then((result) => {
			var streamList = StateStore.getState().streamList;
      streamList.push(result);
      StateStore.dispatch({type: "setStreamList", streamList: streamList});
      StateStore.dispatch({type: "loadStreamAndPlay", stream: result, index: 0});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_play_stream_ok"),
				level: 'info'
			});
			this.setState({show: false});
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_error_play"),
				level: 'error'
			});
    });
	}
	
	getMediaCover() {
    if (this.state.show && this.state.media) {
      this.state.media && StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&base64")
      .then((result) => {
        this.setState({imgBlob: result});
      })
      .fail(() => {
        this.setState({imgBlob: false});
      });
    }
	}
	
	handleSelectDataSource() {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
			StateStore.dispatch({type: "setCurrentPath", path: ""});
		});
	}
	
	handleSelectArtist(value) {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentCategory", category: "artist", categoryValue: value});
		});
	}
	
	handleSelectAlbum(value) {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentCategory", category: "album", categoryValue: value});
		});
	}
	
	handleSelectYear(value) {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentCategory", category: "year", categoryValue: value});
		});
	}
	
	handleSelectGenre(value) {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentCategory", category: "genre", categoryValue: value});
		});
	}
	
	addToStream(stream) {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream) + "/manage", {command: "append_list", parameters: [{data_source: this.state.media.data_source, path: this.state.media.path}]})
    .then((result) => {
      StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream) + "/manage", {command: "info"})
      .then((streamInfo) => {
        StateStore.dispatch({type: "setStream", stream: streamInfo});
        StateStore.getState().NotificationManager.addNotification({
          message: i18n.t("common.message_adding_stream_ok"),
          level: 'info'
        });
      });
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_error_adding_stream"),
				level: 'error'
			});
    });
	}
	
	addToPlaylist(playlist) {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/playlist/" + encodeURIComponent(playlist) + "/add_media", [{data_source: this.state.media.data_source, path: this.state.media.path}])
    .then((result) => {
      StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(playlist))
      .then((newPlaylist) => {
        StateStore.dispatch({type: "setPlaylist", playlist: newPlaylist});
        StateStore.getState().NotificationManager.addNotification({
          message: i18n.t("common.message_adding_stream_ok"),
          level: 'info'
        });
      });
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_error_adding_stream"),
				level: 'error'
			});
    });
	}
	
	addToNewPlaylist() {
		this.setState({addPlaylistShow: true});
	}
	
	onSavePlaylist(result, playlist) {
		this.setState({addPlaylistShow: false}, () => {
			if (result) {
				var parameters;
				if (this.state.path) {
					parameters = {data_source: this.state.dataSource, path: this.state.path, recursive: true};
				} else {
					parameters = {data_source: this.state.dataSource, category: this.state.category, category_value: this.state.categoryValue, sub_category: (this.state.subCategory?this.state.subCategory:undefined), sub_category_value: (this.state.subCategoryValue?this.state.subCategoryValue:undefined) };
				}
				playlist.media = [parameters]
				StateStore.getState().APIManager.taliesinApiRequest("POST", "/playlist/", playlist)
				.then(() => {
					var list = this.state.playlist
					list.push(playlist);
					StateStore.dispatch({type: "setPlaylists", playlists: list});
					this.setState({playlist: list, curPlaylist: false});
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("playlist.message_playlist_added"),
						level: 'info'
					});
				})
				.fail(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("playlist.message_error_adding_playlist"),
						level: 'error'
					});
				});
			}
		});
	}
	
  render() {
		var metadata = [], mediaImage = "", separator = "", streamList = [], playlist = [<MenuItem key={0} onClick={() => this.addToNewPlaylist()}>{i18n.t("common.new_playlist")}</MenuItem>];
		if (this.state.media) {
			if (this.state.media.tags && this.state.media.tags.title) {
				metadata.push(
					<Row key={0}>
						<Col xs={6}>
							<label>{i18n.t("common.title")}</label>
						</Col>
						<Col xs={6}>
							<span>{this.state.media.tags.title}</span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags && this.state.media.tags.artist) {
				metadata.push(
					<Row key={1}>
						<Col xs={6}>
							<label>{i18n.t("common.artist")}</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" onClick={() => {this.handleSelectArtist(this.state.media.tags.artist)}}>{this.state.media.tags.artist}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags && this.state.media.tags.album) {
				metadata.push(
					<Row key={2}>
						<Col xs={6}>
							<label>{i18n.t("common.album")}</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" onClick={() => {this.handleSelectAlbum(this.state.media.tags.album)}}>{this.state.media.tags.album}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags && this.state.media.tags.date) {
				metadata.push(
					<Row key={3}>
						<Col xs={6}>
							<label>{i18n.t("common.date")}</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" onClick={() => {this.handleSelectYear(this.state.media.tags.date)}}>{this.state.media.tags.date}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags && this.state.media.tags.genre) {
				metadata.push(
					<Row key={4}>
						<Col xs={6}>
							<label>{i18n.t("common.genre")}</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" onClick={() => {this.handleSelectGenre(this.state.media.tags.genre)}}>{this.state.media.tags.genre}</a></span>
						</Col>
					</Row>);
			}
			if (metadata.length) {
				separator = <Row><hr/></Row>;
			}
			if (this.state.imgBlob) {
				mediaImage = <Image src={"data:image/jpeg;base64," + this.state.imgBlob} className="cover-image-full center-block" responsive />;
			}
      this.state.streamList.forEach((stream, index) => {
        streamList.push(
          <MenuItem key={index} onClick={() => this.addToStream(stream.name)}>
            - {stream.display_name||i18n.t("common.no_name")}
          </MenuItem>
        );
      });
      this.state.playlist.forEach((pl, index) => {
        playlist.push(
          <MenuItem key={index+1} onClick={() => this.addToPlaylist(pl.name)}>
            - {pl.name}
          </MenuItem>
        );
      });
			return (
					<Modal show={this.state.show} onHide={this.onCloseModal}>
						<Modal.Header closeButton>
							<Modal.Title>
                <ButtonGroup>
                  <Button onClick={this.onPlayNow} className="btn" title={i18n.t("common.play_now")}>
                    <FontAwesome name={"play"} />
                  </Button>
                  <DropdownButton id={"add"} title={
                    <span><i className="fa fa-plus"></i></span>
                  }>
                    <MenuItem>
                      {i18n.t("common.add_to_stream")}
                    </MenuItem>
                    {streamList}
                    <MenuItem divider />
                    <MenuItem>
                      {i18n.t("common.add_to_playlist")}
                    </MenuItem>
                    {playlist}
                  </DropdownButton>
                </ButtonGroup>&nbsp;
								{this.state.title}
							</Modal.Title>
						</Modal.Header>
						<Modal.Body>
							{metadata}
							{separator}
							<Row>
								<Col xs={6}>
									<label>{i18n.t("common.data_source")}</label>
								</Col>
								<Col xs={6}>
									<span><a role="button" onClick={this.handleSelectDataSource}>{this.state.media.data_source}</a></span>
								</Col>
							</Row>
							<Row>
								<Col xs={6}>
									<label>{i18n.t("common.name")}</label>
								</Col>
								<Col xs={6}>
									<span>{this.state.media.name}</span>
								</Col>
							</Row>
							<Row>
								<Col xs={6}>
									<label>{i18n.t("common.path")}</label>
								</Col>
								<Col xs={6}>
									<span>{this.state.media.path}</span>
								</Col>
							</Row>
							<Row>
								<hr/>
							</Row>
							<Row>
								<Col xs={12} className="text-center">
									{mediaImage}
								</Col>
							</Row>
							<Row style={{marginTop: "10px"}}>
								<Col xs={12} className="text-center">
									<Button onClick={this.onCloseModal} className="btn btn-success">
										{i18n.t("common.close")}
									</Button>
								</Col>
							</Row>
						</Modal.Body>
					</Modal>
			);
		} else {
			return (<div></div>);
		}
	}
}

export default ModalMedia;
