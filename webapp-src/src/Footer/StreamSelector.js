import React, { Component } from 'react';
import { DropdownButton, MenuItem, Button, ButtonGroup } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';
import config from '../lib/ConfigManager';

class StreamSelector extends Component {
	constructor(props) {
		super(props);
		this.state = {streamList: props.streamList, selectedStream: props.stream};
		this.handleSelectStream = this.handleSelectStream.bind(this);
		this.handleManageStreams = this.handleManageStreams.bind(this);
		this.handleLoadStream = this.handleLoadStream.bind(this);
		this.handleDetailsStream = this.handleDetailsStream.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({streamList: nextProps.streamList, selectedStream: nextProps.stream});
	}
	
	handleDetailsStream() {
		StateStore.dispatch({type: "setStreamDetails", stream: this.state.selectedStream});
		StateStore.dispatch({type: "loadStream", stream: this.state.selectedStream});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "streamDetails"});
	}

	handleLoadStream() {
		StateStore.dispatch({type: "loadStreamAndPlay", stream: this.state.selectedStream, index: 0});
	}
	
	handleManageStreams() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "manageStream"});
	}
	
	handleSelectStream(name) {
		var newStream = this.state.streamList.find((stream) => {return stream.name === name});
		if (newStream) {
			this.setState({selectedStream: newStream}, () => {
				config.setLocalConfigValue("stream", newStream);
				StateStore.dispatch({type: "loadStream", stream: this.state.selectedStream});
			});
		}
	}
	
	render() {
		var currentList = [];
		this.state.streamList.forEach((stream, index) => {
			currentList.push(
				<MenuItem key={index} eventKey={index} onClick={() => this.handleSelectStream(stream.name)} data-name={stream.name} className={stream.name===this.state.selectedStream.name?"bg-success":""}>{stream.display_name||i18n.t("common.no_name")}</MenuItem>
			)
		});
		var streamName = i18n.t("player.select_stream");
		if (this.state.selectedStream.name) {
			if (this.state.selectedStream.display_name.startsWith("{") && this.state.selectedStream.display_name.indexOf("} - ") !== -1) {
				streamName = this.state.selectedStream.display_name.substring(this.state.selectedStream.display_name.indexOf("}") + 3);
			} else {
				streamName = this.state.selectedStream.display_name||i18n.t("common.no_name");
			}
			if (streamName.length > 10) {
				streamName = streamName.substring(0, 10) + "...";
			}
		}
		return (
			<div>
				<ButtonGroup className="hidden-xs">
					<DropdownButton title={streamName} id="streamList">
						{currentList}
						<MenuItem divider />
						<MenuItem onClick={this.handleManageStreams}>
							{i18n.t("common.manage_streams")}
						</MenuItem>
					</DropdownButton>
					<Button title={i18n.t("common.play_now")} onClick={this.handleLoadStream}>
						<FontAwesome name={"play"} />
					</Button>
					<Button title={i18n.t("common.details")} onClick={this.handleDetailsStream}>
						<FontAwesome name={"eye"} />
					</Button>
				</ButtonGroup>
				<ButtonGroup className="visible-xs">
					<DropdownButton noCaret title={<span><i className="fa fa-list"></i></span>} id="streamListSmall">
						{currentList}
						<MenuItem divider />
						<MenuItem onClick={this.handleManageStreams}>
							{i18n.t("common.manage_streams")}
						</MenuItem>
					</DropdownButton>
					<DropdownButton noCaret title={<span><i className="fa fa-music"></i></span>} id="streamPlayer">
						<MenuItem onClick={this.handleLoadStream}>
							<FontAwesome name={"play"} /> {i18n.t("common.play_now")}
						</MenuItem>
						<MenuItem onClick={this.handleDetailsStream}>
							<FontAwesome name={"eye"} /> {i18n.t("common.details")}
						</MenuItem>
					</DropdownButton>
				</ButtonGroup>
			</div>
		);
	}
}

export default StreamSelector;
