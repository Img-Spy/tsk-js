"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = Object.setPrototypeOf ||
        ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
        function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var __assign = (this && this.__assign) || Object.assign || function(t) {
    for (var s, i = 1, n = arguments.length; i < n; i++) {
        s = arguments[i];
        for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p))
            t[p] = s[p];
    }
    return t;
};
Object.defineProperty(exports, "__esModule", { value: true });
var fs = require("fs");
var path = require("path");
var index_1 = require("typedoc/dist/lib/models/reflections/index");
var UrlMapping_1 = require("typedoc/dist/lib/output/models/UrlMapping");
var DefaultTheme_1 = require("typedoc/dist/lib/output/themes/DefaultTheme");
var flavour_enum_1 = require("./enums/flavour.enum");
var theme_service_1 = require("./theme.service");
var MarkdownTheme = (function (_super) {
    __extends(MarkdownTheme, _super);
    function MarkdownTheme(renderer, basePath, options) {
        var _this = _super.call(this, renderer, basePath) || this;
        renderer.removeComponent('assets');
        renderer.removeComponent('javascript-index');
        renderer.removeComponent('toc');
        renderer.removeComponent('pretty-print');
        theme_service_1.ThemeService.options = {
            mdHideSources: true,
            hideBreadcrumbs: true
        };
        theme_service_1.ThemeService.resources = _this.resources;
        return _this;
    }
    MarkdownTheme.buildUrls = function (reflection, urls) {
        var mapping = DefaultTheme_1.DefaultTheme.getMapping(reflection);
        var options = theme_service_1.ThemeService.options;
        if (mapping) {
            if (!reflection.url || !DefaultTheme_1.DefaultTheme.URL_PREFIX.test(reflection.url)) {
                var url = [mapping.directory, DefaultTheme_1.DefaultTheme.getUrl(reflection) + '.md'].join('/');
                urls.push(new UrlMapping_1.UrlMapping(url, __assign(reflection, { hideBreadcrumbs: options.hideBreadcrumbs }), mapping.template));
                reflection.url = url;
                reflection.hasOwnDocument = true;
            }
            for (var key in reflection.children) {
                if (reflection.children.hasOwnProperty(key)) {
                    var child = reflection.children[key];
                    if (mapping.isLeaf) {
                        MarkdownTheme.applyAnchorUrl(child, reflection);
                    }
                    else {
                        MarkdownTheme.buildUrls(child, urls);
                    }
                }
            }
        }
        else {
            MarkdownTheme.applyAnchorUrl(reflection, reflection.parent);
        }
        return urls;
    };
    MarkdownTheme.applyAnchorUrl = function (reflection, container) {
        var options = theme_service_1.ThemeService.getOptions();
        if (!reflection.url || !DefaultTheme_1.DefaultTheme.URL_PREFIX.test(reflection.url)) {
            var anchor = DefaultTheme_1.DefaultTheme.getUrl(reflection, container, '.');
            if (reflection['isStatic']) {
                anchor = 'static-' + anchor;
            }
            var anchorRef = '';
            switch (reflection.kind) {
                case index_1.ReflectionKind.ExternalModule:
                    anchorRef = "external-module-" + theme_service_1.ThemeService.getAnchorRef(reflection.name) + "-";
                    break;
                case index_1.ReflectionKind.Class:
                    anchorRef = "class-" + theme_service_1.ThemeService.getAnchorRef(reflection.name);
                    break;
                case index_1.ReflectionKind.Interface:
                    anchorRef = "interface-" + theme_service_1.ThemeService.getAnchorRef(reflection.name);
                    break;
                case index_1.ReflectionKind.Module:
                    anchorRef = "module-" + theme_service_1.ThemeService.getAnchorRef(reflection.name);
                case index_1.ReflectionKind.Enum:
                    var isModule = reflection.parent.kind === 0 || reflection.parent.kind === index_1.ReflectionKind.ExternalModule;
                    anchorRef = isModule ?
                        "module-" + theme_service_1.ThemeService.getAnchorRef(reflection.name) :
                        "enumeration-" + theme_service_1.ThemeService.getAnchorRef(reflection.name);
                    break;
                default:
                    if (options.mdFlavour === flavour_enum_1.Flavour.BITBUCKET) {
                        var anchorPrefix_1 = '';
                        if (reflection.kind === index_1.ReflectionKind.ObjectLiteral) {
                            anchorPrefix_1 += 'object-literal-';
                        }
                        reflection.flags.forEach(function (flag) {
                            anchorPrefix_1 += flag + "-";
                        });
                        var prefixRef = theme_service_1.ThemeService.getAnchorRef(anchorPrefix_1);
                        var reflectionRef = theme_service_1.ThemeService.getAnchorRef(reflection.name);
                        anchorRef = "markdown-header-" + prefixRef + reflectionRef;
                    }
                    else {
                        anchorRef = anchor;
                    }
            }
            reflection.url = (container.url !== undefined ? container.url : '') + '#' + anchorRef;
            reflection.anchor = anchor;
            reflection.hasOwnDocument = false;
        }
        reflection.traverse(function (child) {
            if (child instanceof index_1.DeclarationReflection) {
                MarkdownTheme.applyAnchorUrl(child, container);
            }
        });
    };
    MarkdownTheme.prototype.isOutputDirectory = function (outPath) {
        var files = fs.readdirSync(outPath);
        return fs.existsSync(path.join(outPath, 'README.md')) || (files.length === 1 && path.extname(files[0]) === '.md');
    };
    MarkdownTheme.prototype.getUrls = function (project) {
        var options = theme_service_1.ThemeService.getOptions();
        var urlMappings = [];
        var entryPoint = this.getEntryPoint(project);
        theme_service_1.ThemeService.projectName = entryPoint.name;
        urlMappings.push(new UrlMapping_1.UrlMapping('README.md', __assign({}, entryPoint, {
            displayReadme: this.application.options.getValue('readme') !== 'none',
            hideBreadcrumbs: true,
            isIndex: true,
        }), 'reflection.hbs'));
        if (entryPoint.children) {
            entryPoint.children.forEach(function (child) {
                MarkdownTheme.buildUrls(child, urlMappings);
            });
        }
        if (options.mdFlavour === flavour_enum_1.Flavour.GITBOOK) {
            var navigation = this.getNavigation(project).children.map(function (navigationItem) {
                var subNavigation = navigationItem.dedicatedUrls ? navigationItem.dedicatedUrls.map(function (url) {
                    return {
                        title: function () {
                            var urlMapping = urlMappings.find(function (item) {
                                return item.url === url;
                            });
                            return urlMapping ? urlMapping.model.name : null;
                        },
                        url: url,
                    };
                }) : null;
                return __assign({}, navigationItem, { subNavigation: subNavigation });
            });
            urlMappings.push(new UrlMapping_1.UrlMapping('SUMMARY.md', { navigation: navigation }, 'navigation.hbs'));
        }
        return urlMappings;
    };
    return MarkdownTheme;
}(DefaultTheme_1.DefaultTheme));
exports.default = MarkdownTheme;
