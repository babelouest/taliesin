import React, { Component } from 'react';
import { Row } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ElementPathIcon from './ElementPathIcon';
import ElementPathList from './ElementPathList';

class BrowsePath extends Component {
  constructor(props) {
    super(props);
		this.state = {dataSource: props.dataSource, path: props.path, view: props.view, elementList: [], elementListInitial: [], filter: "", loaded: false};
		
		this.getElementList();
		
		this.filterList = this.filterList.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({dataSource: nextProps.dataSource, path: nextProps.path, view: nextProps.view, elementList: [], elementListInitial: [], filter: "", loaded: false}, () => {
			this.getElementList();
		});
	}
	
	filterList(event) {
		var newList = this.state.elementListInitial;
		newList = newList.filter((item) => {
			return item.name.toLowerCase().search(event.target.value.toLowerCase()) !== -1;
		});
		this.setState({elementList: newList});
	}
  
  getElementList() {
		if (!this.state.loaded && this.state.dataSource) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23").replace(/\+/g, "%2B"))
			.then((result) => {
				this.setState({loaded: true, elementListInitial: result, elementList: result});
			})
			.fail((result) => {
				this.setState({loaded: true, elementListInitial: [], elementList: []});
			});
		}
  }
	
	render() {
		var currentList = [];
		var currentElementList = this.state.elementList;
		
		if (this.state.loaded) {
			if (this.state.view === "icon") {
				currentElementList.forEach((element, index) => {
					currentList.push(
						<ElementPathIcon key={index} dataSource={this.state.dataSource} path={this.state.path} element={element} />
					);
				});
				
				return (
					<div>
						<Row>
							<input type="text" placeholder="Filter" value={this.filter} className="form-control" onChange={this.filterList}/>
						</Row>
						<Row>
							{currentList}
						</Row>
					</div>
				);
			} else {
				currentElementList.forEach((element, index) => {
					currentList.push(
						<ElementPathList key={index} dataSource={this.state.dataSource} path={this.state.path} element={element} />
					);
				});
				
				return (
					<div>
						<Row>
							<input type="text" placeholder="Filter" value={this.filter} className="form-control" onChange={this.filterList}/>
						</Row>
						{currentList}
					</div>
				);
			}
		} else {
			return (
				<Row>
					<FontAwesome name="spinner" size="2x" spin />
				</Row>
			);
		}
	}
}

export default BrowsePath;
