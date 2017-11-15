import React, { Component } from 'react';
import FontAwesome from 'react-fontawesome';
import { Image, Col, DropdownButton, MenuItem, ButtonGroup } from 'react-bootstrap';
import VisibilitySensor from 'react-visibility-sensor';
import StateStore from '../lib/StateStore';

class ElementPathIcon extends Component {
  constructor(props) {
    super(props);
		this.state = {element: props.element, dataSource: props.dataSource, path: props.path, thumb: false, thumbLoaded: false, visible: false};
		
		this.handleChangePath = this.handleChangePath.bind(this);
		this.handleOpenFile = this.handleOpenFile.bind(this);
		this.onChangeVisibility = this.onChangeVisibility.bind(this);
		this.getThumbnail = this.getThumbnail.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({element: nextProps.element, dataSource: nextProps.dataSource, path: nextProps.path});
	}
	
	handleChangePath(subPath) {
		StateStore.dispatch({ type: 'setCurrentPath', path: (this.state.path?(this.state.path + "/"):"") + subPath });
	}
  
  handleOpenFile(name) {
  }
	
	onChangeVisibility(isVisible) {
    this.setState({visible: isVisible}, () => {
			if (isVisible && !this.state.thumbLoaded) {
				this.getThumbnail();	
			}
		});
	}
	
	getThumbnail() {
		if (!this.state.thumb && (this.state.element.type === "audio" || this.state.element.type === "folder" || this.state.element.type === "image" || this.state.element.type === "artist" || this.state.element.type === "album" || this.state.element.type === "year" || this.state.element.type === "genre")) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "/" + this.state.element.name + "?cover&thumbnail&base64")
			.then((result) => {
				this.setState({thumb: result, thumbLoaded: true});
			})
			.fail(() => {
				this.setState({thumb: false, thumbLoaded: true});
			});
		} else {
			this.setState({thumb: false, thumbLoaded: true});
		}
	}
	
	render() {
		var icon = "";
		var menu = "";
		if (!this.state.thumbLoaded) {
			icon = 
				<div>
          <a role="button" onClick={() => this.handleChangePath(this.state.element.name)} title={this.state.element.name}>
            <Image src="/images/unknown-128.png" alt={this.state.element.name} className="elementImage" responsive>
            </Image>
            <FontAwesome name="spinner" spin />
            <div className="hideOverflow">
              <span>{this.state.element.name}</span>
            </div>
          </a>
				</div>
		} else {
			if (!this.state.thumb) {
				if (this.state.element.type === "folder") {
					icon =
						<a role="button" onClick={() => this.handleChangePath(this.state.element.name)} title={this.state.element.name}>
							<Image src="/images/folder-128.png" alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else if (this.state.element.type === "audio") {
					icon =
            <a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
              <Image src="/images/audio-128.png" alt={this.state.element.name} className="elementImage" responsive>
              </Image>
              <div className="hideOverflow">
                <span>{this.state.element.name}</span>
              </div>
            </a>
				} else if (this.state.element.type === "artist") {
					icon =
            <a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
              <Image src="/images/artist-128.png" alt={this.state.element.name} className="elementImage" responsive>
              </Image>
              <div className="hideOverflow">
                <span>{this.state.element.name}</span>
              </div>
            </a>
				} else if (this.state.element.type === "album") {
					icon =
            <a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
              <Image src="/images/album-128.png" alt={this.state.element.name} className="elementImage" responsive>
              </Image>
              <div className="hideOverflow">
                <span>{this.state.element.name}</span>
              </div>
            </a>
				} else if (this.state.element.type === "year") {
					icon =
            <a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
              <Image src="/images/year-128.png" alt={this.state.element.name} className="elementImage" responsive>
              </Image>
              <div className="hideOverflow">
                <span>{this.state.element.name}</span>
              </div>
            </a>
				} else if (this.state.element.type === "genre") {
					icon =
            <a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
              <Image src="/images/genre-128.png" alt={this.state.element.name} className="elementImage" responsive>
              </Image>
              <div className="hideOverflow">
                <span>{this.state.element.name}</span>
              </div>
            </a>
				} else {
					icon =
            <a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
              <Image src="/images/unknown-128.png" alt={this.state.element.name} className="elementImage" responsive>
              </Image>
              <div className="hideOverflow">
                <span>{this.state.element.name}</span>
              </div>
            </a>
				}
			} else {
				if (this.state.element.type === "folder" || this.state.element.type === "artist" || this.state.element.type === "album" || this.state.element.type === "year" || this.state.element.type === "genre") {
					icon =
						<a role="button" onClick={() => this.handleChangePath(this.state.element.name)} title={this.state.element.name}>
							<Image src={"data:image/jpeg;base64,"+this.state.thumb} alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else if (this.state.element.type === "audio") {
					icon =
            <a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
              <Image src={"data:image/jpeg;base64,"+this.state.thumb} alt={this.state.element.name} className="elementImage" responsive>
              </Image>
              <div className="hideOverflow">
                <span>{this.state.element.name}</span>
              </div>
            </a>
				} else {
					icon =
            <a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
              <Image src={"data:image/jpeg;base64,"+this.state.thumb} alt={this.state.element.name} className="elementImage" responsive>
              </Image>
              <div className="hideOverflow">
                <span>{this.state.element.name}</span>
              </div>
            </a>
				}
			}
		}
		if (this.state.element.type === "folder") {
			menu = 
				<ButtonGroup>
					<DropdownButton id={"play"-this.state.element.name} title={
						<span><i className="fa fa-play"></i></span>
					}>
						<MenuItem eventKey="1">As a webradio (no random)</MenuItem>
						<MenuItem eventKey="2">As a webradio (random)</MenuItem>
						<MenuItem eventKey="3">As a jukebox</MenuItem>
					</DropdownButton>
					<DropdownButton id={"add"-this.state.element.name} title={
						<span><i className="fa fa-plus"></i></span>
					}>
						<MenuItem eventKey="1">Add to playlist</MenuItem>
						<MenuItem eventKey="2">Add to stream</MenuItem>
					</DropdownButton>
				</ButtonGroup>
		} else if (this.state.element.type === "audio") {
			menu = 
				<ButtonGroup>
					<DropdownButton id={"play"-this.state.element.name} title={
						<span><i className="fa fa-play"></i></span>
					}>
						<MenuItem eventKey="1">As a webradio</MenuItem>
						<MenuItem eventKey="3">As a jukebox</MenuItem>
					</DropdownButton>
					<DropdownButton id={"add"-this.state.element.name} title={
						<span><i className="fa fa-plus"></i></span>
					}>
						<MenuItem eventKey="1">Add to playlist</MenuItem>
						<MenuItem eventKey="2">Add to stream</MenuItem>
					</DropdownButton>
				</ButtonGroup>
		}
		return (
			<Col xs={3} md={2} className="panel panel-default">
        <VisibilitySensor
          scrollCheck
          scrollThrottle={100}
          intervalDelay={8000}
          onChange={this.onChangeVisibility}
          minTopValue={10}
          partialVisibility={true}>
					{icon}
				</VisibilitySensor>
				<div className="text-center">
					{menu}
				</div>
			</Col>
		);
	}
}

export default ElementPathIcon;
