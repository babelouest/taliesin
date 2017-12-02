import React, { Component } from 'react';
import FontAwesome from 'react-fontawesome';
import { Image, Col } from 'react-bootstrap';
import VisibilitySensor from 'react-visibility-sensor';
import ModalMedia from '../Modal/ModalMedia';
import StateStore from '../lib/StateStore';
import ElementButtons from './ElementButtons';

class ElementCategoryIcon extends Component {
  constructor(props) {
    super(props);
		this.state = {
			dataSource: props.dataSource, 
			category: props.category, 
			categoryValue: props.categoryValue, 
			subCategory: props.subCategory, 
			subCategoryValue: props.subCategoryValue, 
			element: props.element,
			modal: ""
		};
		
		this.handleChangePath = this.handleChangePath.bind(this);
		this.onChangeVisibility = this.onChangeVisibility.bind(this);
		this.getThumbnail = this.getThumbnail.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			dataSource: nextProps.dataSource, 
			category: nextProps.category, 
			categoryValue: nextProps.categoryValue, 
			subCategory: nextProps.subCategory, 
			subCategoryValue: nextProps.subCategoryValue, 
			element: nextProps.element,
			modal: ""
		});
	}
	
	handleChangePath(name) {
		if (this.state.element.type === "media") {
			this.setState({modal: <ModalMedia show={true} media={this.state.element} title={this.state.element.tags.title||this.state.element.name} />});
		} else {
			if (this.state.subCategory) {
				StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: this.state.categoryValue, subCategory: this.state.subCategory, subCategoryValue: name});
			} else {
				StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: name});
			}
		}
	}
	
	onChangeVisibility(isVisible) {
    this.setState({visible: isVisible}, () => {
			if (isVisible && !this.state.thumbLoaded) {
				this.getThumbnail();	
			}
		});
	}
	
	getThumbnail() {
		if (!this.state.thumb) {
			var url;
			if (this.state.element.type !== "media") {
				url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/info/category/" + encodeURI(this.state.category);
				if (this.state.categoryValue) {
					url += "/" + encodeURIComponent(this.state.categoryValue);
					if (this.state.subCategory) {
						url += "/" + encodeURIComponent(this.state.subCategory);
					}
				}
				url += "/" + encodeURIComponent(this.state.element.name);
			} else {
				url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.element.path).replace(/#/g, "%23").replace(/\+/g, "%2B");
			}
			url += "?cover&thumbnail&base64";
			StateStore.getState().APIManager.taliesinApiRequest("GET", url)
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
				icon =
					<a role="button" onClick={() => this.handleChangePath(this.state.element.name)} title={this.state.element.name}>
						<Image src={"/images/" + this.state.element.type + "-128.png"} alt={this.state.element.name} className="elementImage" responsive>
						</Image>
						<div className="hideOverflow">
							<span>{this.state.element.name}</span>
						</div>
					</a>
			} else {
				icon =
					<a role="button" onClick={() => this.handleChangePath(this.state.element.name)} title={this.state.element.name}>
						<Image src={"data:image/jpeg;base64,"+this.state.thumb} alt={this.state.element.name} className="elementImage" responsive>
						</Image>
						<div className="hideOverflow">
							<span>{this.state.element.name}</span>
						</div>
					</a>
			}
		}
		return (
			<Col md={3} sm={3} xs={6} className="panel panel-default">
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
					<ElementButtons dataSource={this.state.dataSource} path={this.state.element.path} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} element={this.state.element}/>
				</div>
				{this.state.modal}
			</Col>
		);
	}
}

export default ElementCategoryIcon;
