import { Component, isDevMode, OnInit, ViewChild } from '@angular/core';
import { Language, MythLanguageList } from "src/app/services/interfaces/language.interface";
import { Theme } from 'src/app/services/interfaces/theme.interface';
import { ThemeService } from '../../services/theme.service';
import { ConfigService } from '../../services/config.service';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { MenuItem, SharedModule } from 'primeng/api';
import { PrimeNG } from 'primeng/config';
import { DataService } from 'src/app/services/data.service';
import { MythService } from 'src/app/services/myth.service';
import { Router } from '@angular/router';
import { MessageModule } from 'primeng/message';
import { CheckboxModule } from 'primeng/checkbox';
import { PasswordModule } from 'primeng/password';
import { DialogModule } from 'primeng/dialog';
import { FormsModule } from '@angular/forms';
import { TableModule } from 'primeng/table';
import { TooltipModule } from 'primeng/tooltip';
import { ButtonModule } from 'primeng/button';
import { RippleModule } from 'primeng/ripple';
import { NgIf } from '@angular/common';
import { Popover, PopoverModule } from 'primeng/popover';
import { Menu, MenuModule } from 'primeng/menu';


@Component({
    selector: 'app-navbar',
    templateUrl: './navbar.component.html',
    styleUrls: ['./navbar.component.css'],
    standalone: true,
    imports: [NgIf, RippleModule, ButtonModule, TooltipModule, PopoverModule, SharedModule, TableModule, FormsModule, DialogModule, PasswordModule, CheckboxModule, MessageModule, TranslateModule, MenuModule]
})
export class NavbarComponent implements OnInit {
    @ViewChild("themePanel") themePanel!: Popover;
    @ViewChild("languagePanel") languagePanel!: Popover;
    @ViewChild("menu") menu!: Menu;

    m_themes$!: Theme[];
    m_selectedTheme!: Theme;

    m_languages!: Language[];
    m_selectedLanguage!: Language;

    m_showNavbar: boolean = true;
    showTopBar = true;
    m_devMode: boolean = isDevMode();
    m_haveDatabase: boolean = true;
    userName: string | null = '';
    userPassword: string | null = '';
    displayLogin = false;
    errorCount = 0;
    APIAuthReqd = false;
    keepLogin = false;
    navMenu: MenuItem[] = [];
    mnulogin: MenuItem = { id: 'login', label: 'navbar.login', command: (event) => this.showLogin() };
    mnulogout: MenuItem = { id: 'logout', label: 'navbar.logout', command: (event) => this.logout() };
    // { label: 'navbar.switchTheme', command: (event) => this.themePanel.toggle(event) },
    mnulang: MenuItem = {
        label: 'navbar.changeLanguage',
        command: (event) => { this.languagePanel.toggle(event.originalEvent) }
    };

    constructor(private themeService: ThemeService,
        private configService: ConfigService,
        private translateService: TranslateService,
        private primeconfigService: PrimeNG,
        public dataService: DataService,
        private mythService: MythService,
        private router: Router) {
        // this.themeService.getThemes().then((themes: Theme[]) => {
        //     this.m_themes$ = themes;
        //     this.m_selectedTheme = this.findThemeByName(localStorage.getItem('Theme') || 'Blue Light');
        //     this.themeService.switchTheme(this.m_selectedTheme.CSS);
        // });

        this.configService.GetLanguages().subscribe((data: MythLanguageList) => {
            this.m_languages = data.LanguageList.Languages;
            this.m_selectedLanguage = this.findLanguageByCode(localStorage.getItem('Language') || 'en_US');
            // if this was successful without being logged in, reset Auth Reqd
            if (!sessionStorage.getItem('loggedInUser')) {
                sessionStorage.removeItem('APIAuthReqd')
                this.closeLogin();
            }
        })

        this.mythService.GetBackendInfo()
            .subscribe(
                data => {
                    var url = this.router.url;
                    if (data.BackendInfo.Env.IsDatabaseIgnored
                        || (!data.BackendInfo.Env.SchedulingEnabled && !url.startsWith('/setupwizard/')))
                        router.navigate(['setupwizard/dbsetup']);
                    else if (url == '/')
                        router.navigate(['dashboard/status']);
                });

        [this.mnulogin,this.mnulogout,this.mnulang].forEach(entry => {
            if (entry.label)
                this.translateService.get(entry.label).subscribe(data =>
                    entry.label = data
                );
        });
    }

    ngOnInit(): void {
        this.dataService.loggedInUser = sessionStorage.getItem('loggedInUser');
        this.APIAuthReqd = (sessionStorage.getItem('APIAuthReqd') != null);
        if (!this.dataService.loggedInUser) {
            this.userName = localStorage.getItem('userName');
            this.userPassword = localStorage.getItem('userPassword');
            if (this.userName && this.userPassword) {
                this.login();
                return;
            }
            if (this.APIAuthReqd) {
                this.displayLogin = true;
                return;
            }
        }
    }

    findThemeByName(name: string): Theme {
        for (var x = 0; x < this.m_themes$.length; x++) {
            if (this.m_themes$[x].Name === name)
                return this.m_themes$[x];

        }

        return this.m_themes$[0];
    }

    changeTheme(theme: Theme) {
        // this.m_selectedTheme = theme;
        // this.themeService.switchTheme(theme.CSS);
        // localStorage.setItem('Theme', this.m_selectedTheme.Name);
    }

    findLanguageByCode(code: string): Language {
        for (var x = 0; x < this.m_languages.length; x++) {
            if (this.m_languages[x].Code === code)
                return this.m_languages[x];

        }

        return this.m_languages[0];
    }

    changeLanguage(language: Language) {
        console.log("Language changed to ", language.NativeLanguage)
        this.m_selectedLanguage = language;
        localStorage.setItem('Language', this.m_selectedLanguage.Code);
        this.translateService.use(this.m_selectedLanguage.Code);
        this.translateService.get('primeng').subscribe(res => this.primeconfigService.setTranslation(res));
    }

    toggleShowNavbar() {
        this.m_showNavbar = !this.m_showNavbar;
    }

    showMenu(event: any) {
        this.navMenu = [];
        if (this.dataService.loggedInUser)
            this.navMenu.push(this.mnulogout);
        else
            this.navMenu.push(this.mnulogin);
        this.navMenu.push(this.mnulang);
        this.menu.show(event);
    }

    toggleShowSidebar() {
        this.dataService.toggleShowSidebar();
    }

    showLogin() {
        this.APIAuthReqd = (sessionStorage.getItem('APIAuthReqd') != null);
        this.displayLogin = true;
    }

    login() {
        // login process
        this.errorCount = 0;
        this.mythService.LoginUser(this.userName!, this.userPassword!).subscribe(
            {
                next: (data: any) => {
                    if (data.String) {
                        sessionStorage.setItem('accessToken', data.String);
                        sessionStorage.setItem('loggedInUser', this.userName!);
                        if (this.keepLogin) {
                            localStorage.setItem('userName', this.userName!)
                            localStorage.setItem('userPassword', this.userPassword!)
                        }
                        location.reload();
                    }
                    else {
                        this.displayLogin = true;
                        this.errorCount++;
                    }
                },
                error: (err: any) => {
                    console.log("Login error", err);
                    this.errorCount++;
                }
            });
    }

    closeLogin() {
        this.displayLogin = false;
    }

    logout() {
        this.dataService.loggedInUser = '';
        sessionStorage.removeItem('accessToken');
        sessionStorage.removeItem('loggedInUser');
        localStorage.removeItem('userName');
        localStorage.removeItem('userPassword');
        location.reload();
    }
}
